/*
 * Copyright (c) 2014-2015 brc developers (see AUTHORS)
 *
 * This file is part of brc.
 *
 * brc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "btcnet.hpp"

#include <czmq++/czmqpp.hpp>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

void log_to_file(std::ofstream& file,
    bc::log_level level, const std::string& domain, const std::string& body)
{
    if (body.empty())
        return;
    file << level_repr(level);
    if (!domain.empty())
        file << " [" << domain << "]";
    file << ": " << body << std::endl;
}
void log_to_both(std::ostream& device, std::ofstream& file,
    bc::log_level level, const std::string& domain, const std::string& body)
{
    if (body.empty())
        return;
    std::ostringstream output;
    output << level_repr(level);
    if (!domain.empty())
        output << " [" << domain << "]";
    output << ": " << body;
    device << output.str() << std::endl;
    file << output.str() << std::endl;
}

void output_file(std::ofstream& file, bc::log_level level,
    const std::string& domain, const std::string& body)
{
    log_to_file(file, level, domain, body);
}
void output_both(std::ofstream& file, bc::log_level level,
    const std::string& domain, const std::string& body)
{
    log_to_both(std::cout, file, level, domain, body);
}

void warning(std::ofstream& file, bc::log_level level,
    const std::string& domain, const std::string& body)
{
    if (domain == LOG_BRC)
        log_to_both(std::cerr, file, level, domain, body);
    else
        log_to_file(file, level, domain, body);
}
void error(std::ofstream& file, bc::log_level level,
    const std::string& domain, const std::string& body)
{
    log_to_both(std::cerr, file, level, domain, body);
}

broadcaster::broadcaster()
  : hosts_(pool_), handshake_(pool_), network_(pool_),
    broadcast_p2p_(
        pool_, hosts_, handshake_, network_)
{
}

void broadcaster::start(size_t threads,
    size_t broadcast_hosts, start_handler handle_start)
{
    outfile_.open("debug.log");
    errfile_.open("error.log");
    // Suppress noisy output.
    bc::log_debug().set_output_function(
        std::bind(output_file, std::ref(outfile_), _1, _2, _3));
    bc::log_info().set_output_function(
        std::bind(output_both, std::ref(outfile_), _1, _2, _3));
    bc::log_warning().set_output_function(
        std::bind(warning, std::ref(errfile_), _1, _2, _3));
    bc::log_error().set_output_function(
        std::bind(error, std::ref(errfile_), _1, _2, _3));
    bc::log_fatal().set_output_function(
        std::bind(error, std::ref(errfile_), _1, _2, _3));
    // Begin initialization.
    pool_.spawn(threads);
    // Set connection counts.
    broadcast_p2p_.set_max_outbound(broadcast_hosts);
    // Start connecting to p2p network for broadcasting txs.
    broadcast_p2p_.start(handle_start);
}
void broadcaster::stop()
{
    pool_.stop();
    pool_.join();
}

struct summary_stats
{
    size_t current()
    {
        return success + failure;
    }

    size_t success = 0;
    size_t failure = 0;
};

typedef std::shared_ptr<summary_stats> summary_stats_ptr;

void send_summary(const bc::hash_digest& tx_hash, const summary_stats& stats)
{
    bc::log_info(LOG_BRC) << "Sending summary for: "
        << bc::encode_hash(tx_hash);
    // Create ZMQ socket.
    static czmqpp::context ctx;
    static czmqpp::authenticator auth(ctx);
    static czmqpp::socket socket(ctx, ZMQ_PUB);
    static bool is_initialized = false;
    if (!is_initialized)
    {
#ifdef ONLY_LOCALHOST_CONNECTIONS
        auth.allow("127.0.0.1");
#endif
        bc::log_debug(LOG_BRC) << "Initializing summary socket.";
        BITCOIN_ASSERT(ctx.self());
        BITCOIN_ASSERT(socket.self());
        int bind_rc = socket.bind(listen_transport(publish_summary_port));
        BITCOIN_ASSERT(bind_rc != -1);
        is_initialized = true;
        sleep(1);
    }
    // Create a message.
    czmqpp::message msg;
    // tx_hash
    const czmqpp::data_chunk data_hash(tx_hash.begin(), tx_hash.end());
    msg.append(data_hash);
    // Success
    const auto data_success = bc::to_little_endian(stats.success);
    msg.append(bc::to_data_chunk(data_success));
    // Failure
    const auto data_failure = bc::to_little_endian(stats.failure);
    msg.append(bc::to_data_chunk(data_failure));
    // Send it.
    bool rc = msg.send(socket);
    BITCOIN_ASSERT(rc);
}

bool broadcaster::broadcast(const bc::data_chunk& raw_tx)
{
    bc::transaction_type tx;
    try
    {
        satoshi_load(raw_tx.begin(), raw_tx.end(), tx);
    }
    catch (bc::end_of_stream)
    {
        bc::log_warning(LOG_BRC) << "Bad stream.";
        return false;
    }
    bc::hash_digest tx_hash = bc::hash_transaction(tx);
    bc::log_info(LOG_BRC) << "Broadcasting: " << bc::encode_hash(tx_hash);
    // Summary stats
    summary_stats_ptr stats = std::make_shared<summary_stats>();
    // We can ignore the send since we have connections to monitor
    // from the network and resend if neccessary anyway.
    auto tally_send = [tx_hash, stats](
        const std::error_code& ec, size_t total_connections)
    {
        // Increment respective counters for summary statistics.
        if (ec)
            ++stats->failure;
        else
            ++stats->success;
        // Once reach the end, send out summary information.
        if (stats->current() == total_connections)
            send_summary(tx_hash, *stats);
    };
    broadcast_p2p_.broadcast(tx, tally_send);
    return true;
}

size_t broadcaster::total_connections() const
{
    return broadcast_p2p_.total_connections();
}

