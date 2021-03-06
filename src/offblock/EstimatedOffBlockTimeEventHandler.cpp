#include "pch/stdafx.h"
#include "offblock/EstimatedOffBlockTimeEventHandler.h"
#include "euroscope/EuroScopeCFlightPlanInterface.h"
#include "euroscope/EuroScopeCRadarTargetInterface.h"
#include "flightplan/StoredFlightplanCollection.h"
#include "flightplan/StoredFlightplan.h"

using UKControllerPlugin::Flightplan::StoredFlightplanCollection;

namespace UKControllerPlugin {
namespace Datablock {

EstimatedOffBlockTimeEventHandler::EstimatedOffBlockTimeEventHandler(
    const StoredFlightplanCollection & storedFlightplans
) : storedFlightplans(storedFlightplans), displayTime()
{

}

std::string EstimatedOffBlockTimeEventHandler::GetTagItemDescription(void) const
{
    return "Estimated Off-block Time";
}

std::string EstimatedOffBlockTimeEventHandler::GetTagItemData(
    UKControllerPlugin::Euroscope::EuroScopeCFlightPlanInterface & flightPlan,
    UKControllerPlugin::Euroscope::EuroScopeCRadarTargetInterface & radarTarget
) {
    if (!this->storedFlightplans.HasFlightplanForCallsign(flightPlan.GetCallsign())) {
        return this->noTime;
    }

    std::chrono::system_clock::time_point eobt = this->storedFlightplans
        .GetFlightplanForCallsign(flightPlan.GetCallsign())
        .GetExpectedOffBlockTime();

    // If no valid EOBT, nothing to do
    if (eobt == (std::chrono::system_clock::time_point::max)()) {
        return this->noTime;
    }

    return this->displayTime.FromTimePoint(eobt);
}

}  // namespace Datablock
}  // namespace UKControllerPlugin
