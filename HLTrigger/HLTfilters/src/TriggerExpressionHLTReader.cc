#include <cassert>
#include <boost/foreach.hpp>

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Utilities/interface/RegexMatch.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "HLTrigger/HLTfilters/interface/TriggerExpressionHLTReader.h"
#include "HLTrigger/HLTfilters/interface/TriggerExpressionData.h"

namespace triggerExpression {

// define the result of the module from the HLT reults
bool HLTReader::operator()(const Data & data) const {
  if (not data.hasHLT())
    return false;

  typedef std::pair<std::string, unsigned int> value_type;
  BOOST_FOREACH(const value_type & trigger, m_triggers)
    if (data.hltResults().accept(trigger.second))
      return true;
  
  return false;
}

void HLTReader::dump(std::ostream & out) const {
  if (m_triggers.size() == 0) {
    out << "FALSE";
  } else if (m_triggers.size() == 1) {
    out << m_triggers[0].first;
  } else {
    out << "(" << m_triggers[0].first;
    for (unsigned int i = 1; i < m_triggers.size(); ++i)
      out << " OR " << m_triggers[i].first;
    out << ")";
  }
}

// (re)initialize the module
void HLTReader::init(const Data & data) {
  // clear the previous configuration
  m_triggers.clear();

  // check if the pattern has is a glob expression, or a single trigger name
  const edm::TriggerNames & hltMenu = data.hltMenu();
  if (not edm::is_glob(m_pattern)) {
    // no wildcard expression
    unsigned int index = hltMenu.triggerIndex(m_pattern);
    if (index < hltMenu.size())
      m_triggers.push_back( std::make_pair(m_pattern, index) );
    else
      if (data.shouldThrow())
        throw cms::Exception("Configuration") << "requested HLT path \"" << m_pattern << "\" does not exist";
      else
        edm::LogWarning("Configuration") << "requested HLT path \"" << m_pattern << "\" does not exist";
  } else {
    // expand wildcards in the pattern
    const std::vector< std::vector<std::string>::const_iterator > & matches = edm::regexMatch(hltMenu.triggerNames(), m_pattern);
    if (matches.empty()) {
      // m_pattern does not match any trigger paths
      if (data.shouldThrow())
        throw cms::Exception("Configuration") << "requested m_pattern \"" << m_pattern <<  "\" does not match any HLT paths";
      else
        edm::LogWarning("Configuration") << "requested m_pattern \"" << m_pattern <<  "\" does not match any HLT paths";
    } else {
      // store indices corresponding to the matching triggers
      BOOST_FOREACH(const std::vector<std::string>::const_iterator & match, matches) {
        unsigned int index = hltMenu.triggerIndex(*match);
        assert(index < hltMenu.size());
        m_triggers.push_back( std::make_pair(*match, index) );
      }
    }
  }
}

} // namespace triggerExpression
