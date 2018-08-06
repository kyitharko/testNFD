#include "ttt.hpp"
#include "core/logger.hpp"

namespace nfd {

NFD_LOG_INIT("Ttt");

std::ostream&
operator<<(std::ostream& os, TttPacketDecision decision)
{
  switch (decision) {
    case TttPacketDecision::DROP: return os << "DROP";
    case TttPacketDecision::FIB: return os << "FIB";
    case TttPacketDecision::PIT: return os << "PIT";
    case TttPacketDecision::CS: return os << "CS";
    default: return os << "NONE";
  }
}

std::ostream&
operator<<(std::ostream& os, TttTableAction action)
{
  switch (action) {
    case TttTableAction::INS: return os << "INS";
    case TttTableAction::DEL: return os << "DEL";
    default: return os << "NONE";
  }
}

std::ostream&
operator<<(std::ostream& os, TttTable table)
{
  switch (table) {
    case TttTable::FIB: return os << "FIB";
    case TttTable::PIT: return os << "PIT";
    case TttTable::CS: return os << "CS";
    default: return os << "NONE";
  }
}

Ttt&
Ttt::get()
{
  static Ttt instance;
  return instance;
}

Ttt::Ttt()
{
  if (const char* envFace = std::getenv("TTT_FACE")) {
    m_ndnNic = FaceUri(envFace);
  }
  else {
    m_ndnNic = FaceUri("invalid://");
    NFD_LOG_ERROR("TTT_FACE environ is unset, trace will be inaccurate");
  }

  if (const char* envEpoch = std::getenv("TTT_EPOCH")) {
    std::time_t epoch = static_cast<std::time_t>(std::atoll(envEpoch));
    m_epoch = time::system_clock::from_time_t(epoch);
  }
  else {
    m_epoch = time::system_clock::from_time_t(0);
    NFD_LOG_ERROR("TTT_EPOCH environ is unset, timestamps will use UNIX epoch");
  }

  if (const char* envLog = std::getenv("TTT_LOG")) {
    m_os.open(envLog);
    NFD_LOG_INFO("writing trace to " << envLog << " using epoch " << time::toString(m_epoch));
  }
  else {
    NFD_LOG_ERROR("TTT_LOG environ is unset, no trace will be written");
  }
}

bool
Ttt::isNdnNic(const Face& face)
{
  return face.getRemoteUri() == Ttt::get().m_ndnNic;
}

std::ostream&
Ttt::startRecord()
{
  auto timestamp = time::duration_cast<time::microseconds>(time::system_clock::now() - m_epoch).count();
  static char timestampStr[32];
  std::snprintf(timestampStr, sizeof(timestampStr), "%ld.%06ld", (timestamp / 1000000), (timestamp % 1000000));
  return m_os << timestampStr << DELIM;
}

static const Name LOCALHOST("/localhost");

void
Ttt::recordPacketArrival(char pktType, const Name& name, size_t pktSize, TttPacketDecision decision)
{
  if (LOCALHOST.isPrefixOf(name)) {
    return;
  }

  Ttt::get().startRecord() <<
    "PKT" << DELIM <<
    pktType << DELIM <<
    name << DELIM <<
    pktSize << DELIM <<
    decision << std::endl;
}

void
Ttt::recordTableChange(TttTableAction action, TttTable table, const Name& name)
{
  if (LOCALHOST.isPrefixOf(name)) {
    return;
  }

  Ttt::get().startRecord() <<
    action << DELIM <<
    table << DELIM <<
    name << std::endl;
}

} // namespace nfd
