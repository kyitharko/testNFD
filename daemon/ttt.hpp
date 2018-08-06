#ifndef NFD_DAEMON_TTT_HPP
#define NFD_DAEMON_TTT_HPP

#include "face/face.hpp"
#include <fstream>

namespace nfd {

enum class TttPacketDecision {
  NONE,
  DROP,
  FIB,
  PIT,
  CS
};

std::ostream&
operator<<(std::ostream& os, TttPacketDecision decision);

enum class TttTableAction {
  NONE,
  INS,
  DEL
};

std::ostream&
operator<<(std::ostream& os, TttTableAction action);

enum class TttTable {
  NONE,
  FIB,
  PIT,
  CS
};

std::ostream&
operator<<(std::ostream& os, TttTable table);

/** \brief write Traffic and Table Trace for NDN-NIC simulation
 *  \sa https://github.com/yoursunny/NDN-NIC/blob/master/spec/traffic-table-trace.md
 *
 *  To enable writing Traffic and Table Trace, set environment variables:
 *  TTT_FACE is the remote FaceUri of NDN-NIC.
 *  TTT_EPOCH is the scenario start time in `date +%s` format.
 *  TTT_LOG is the output filename.
 */
class Ttt
{
public:
  static bool
  isNdnNic(const Face& face);

  static void
  recordPacketArrival(const Interest& interest, TttPacketDecision decision)
  {
    recordPacketArrival('I', interest.getName(), interest.wireEncode().size(), decision);
  }

  static void
  recordPacketArrival(const Data& data, TttPacketDecision decision)
  {
    recordPacketArrival('D', data.getName(), data.wireEncode().size(), decision);
  }

  static void
  recordTableChange(TttTableAction action, TttTable table, const Name& name);

private:
  Ttt();

  static Ttt&
  get();

  /** \brief write timestamp
   *  \return the output stream
   */
  std::ostream&
  startRecord();

  static void
  recordPacketArrival(char pktType, const Name& name, size_t pktSize, TttPacketDecision decision);

private:
  static const char DELIM = '\t';
  time::system_clock::TimePoint m_epoch;
  std::ofstream m_os;
  FaceUri m_ndnNic;
};

} // namespace nfd

#endif // NFD_DAEMON_TTT_HPP
