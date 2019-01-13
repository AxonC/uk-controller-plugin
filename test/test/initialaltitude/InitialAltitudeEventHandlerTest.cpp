#include "pch/pch.h"
#include "initialaltitude/InitialAltitudeEventHandler.h"
#include "mock/MockEuroScopeCFlightplanInterface.h"
#include "mock/MockEuroScopeCRadarTargetInterface.h"
#include "initialaltitude/InitialAltitudeGenerator.h"
#include "airfield/AirfieldOwnershipManager.h"
#include "controller/ActiveCallsignCollection.h"
#include "airfield/AirfieldCollection.h"
#include "controller/ActiveCallsign.h"
#include "controller/ControllerPosition.h"
#include "airfield/Airfield.h"
#include "login/Login.h"
#include "mock/MockEuroscopePluginLoopbackInterface.h"
#include "timedevent/DeferredEventHandler.h"
#include "controller/ControllerStatusEventHandlerCollection.h"

using UKControllerPlugin::InitialAltitude::InitialAltitudeEventHandler;
using UKControllerPluginTest::Euroscope::MockEuroScopeCFlightPlanInterface;
using UKControllerPluginTest::Euroscope::MockEuroScopeCRadarTargetInterface;
using UKControllerPlugin::InitialAltitude::InitialAltitudeGenerator;
using UKControllerPlugin::Airfield::AirfieldOwnershipManager;
using UKControllerPlugin::Controller::ActiveCallsignCollection;
using UKControllerPlugin::Controller::ActiveCallsign;
using UKControllerPlugin::Airfield::AirfieldCollection;
using UKControllerPlugin::Controller::ControllerPosition;
using UKControllerPlugin::Airfield::Airfield;
using UKControllerPluginTest::Euroscope::MockEuroscopePluginLoopbackInterface;
using UKControllerPlugin::Controller::Login;
using UKControllerPlugin::TimedEvent::DeferredEventHandler;
using UKControllerPlugin::Controller::ControllerStatusEventHandlerCollection;

using ::testing::Test;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::NiceMock;

namespace UKControllerPluginTest {
    namespace InitialAltitude {

        class InitialAltitudeEventHandlerTest : public Test {
            public:
                InitialAltitudeEventHandlerTest()
                    :  owners(airfields, callsigns),
                    login(plugin, ControllerStatusEventHandlerCollection()),
                    handler(generator, callsigns, owners, login, deferredEvents, plugin)
                {

                }

                virtual void SetUp() {
                    // Pretend we've been logged in a while
                    login.SetLoginTime(std::chrono::system_clock::now() - std::chrono::minutes(15));
                    generator.AddSid("EGKK", "ADMAG2X", 6000);
                    ON_CALL(mockFlightPlan, GetCallsign())
                        .WillByDefault(Return("BAW123"));
                }

                AirfieldCollection airfields;
                DeferredEventHandler deferredEvents;
                NiceMock<MockEuroScopeCFlightPlanInterface> mockFlightPlan;
                NiceMock<MockEuroScopeCRadarTargetInterface> mockRadarTarget;
                NiceMock<MockEuroscopePluginLoopbackInterface> plugin;
                Login login;
                ActiveCallsignCollection callsigns;
                AirfieldOwnershipManager owners;
                InitialAltitudeGenerator generator;
                InitialAltitudeEventHandler handler;
        };

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDefersIfNotLoggedInLongEnough)
        {
            login.SetLoginTime(std::chrono::system_clock::now() + std::chrono::minutes(15));
            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
            EXPECT_EQ(1, deferredEvents.Count());
            int64_t seconds = std::chrono::duration_cast<std::chrono::seconds> (
                this->deferredEvents.NextEventTime() - std::chrono::system_clock::now()
                )
                .count();
            EXPECT_LE(seconds, 5);
            EXPECT_GT(seconds, 3);
        }


        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfTooFarFromOrigin)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin + 1));
            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfTravellingTooFast)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed + 1));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfAlreadyHasAClearedAltitude)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(true));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfTracked)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(true));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfAircraftIsSimulated)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(true));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfUserCallsignIsNotActive)
        {
            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, GetOrigin())
                .Times(1)
                .WillOnce(Return("EGKK"));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfAirfieldIsNotOwnedByUser)
        {
            ControllerPosition controller("LON_S_CTR", 129.420, "CTR", { "EGKK" });
            ActiveCallsign userCallsign(
                "LON_S_CTR",
                "Test",
                controller
            );

            callsigns.AddUserCallsign(userCallsign);

            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, GetOrigin())
                .Times(1)
                .WillOnce(Return("EGKK"));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventDoesNotAssignIfSidNotFound)
        {
            ControllerPosition controller("LON_S_CTR", 129.420, "CTR", { "EGKK" });
            ActiveCallsign userCallsign(
                "LON_S_CTR",
                "Test",
                controller
            );
            callsigns.AddUserCallsign(userCallsign);

            airfields.AddAirfield(std::unique_ptr<Airfield>(new Airfield("EGKK", { "LON_S_CTR" })));
            owners.RefreshOwner("EGKK");

            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, GetSidName())
                .Times(1)
                .WillOnce(Return("ADMAG1X"));

            EXPECT_CALL(mockFlightPlan, GetOrigin())
                .Times(2)
                .WillRepeatedly(Return("EGKK"));

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventAssignsIfSidFound)
        {
            ControllerPosition controller("LON_S_CTR", 129.420, "CTR", { "EGKK" });
            ActiveCallsign userCallsign(
                "LON_S_CTR",
                "Test",
                controller
            );
            callsigns.AddUserCallsign(userCallsign);

            airfields.AddAirfield(std::unique_ptr<Airfield>(new Airfield("EGKK", { "LON_S_CTR" })));
            owners.RefreshOwner("EGKK");

            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, GetSidName())
                .Times(2)
                .WillRepeatedly(Return("ADMAG2X"));

            EXPECT_CALL(mockFlightPlan, GetOrigin())
                .Times(4)
                .WillRepeatedly(Return("EGKK"));

            EXPECT_CALL(mockFlightPlan, GetCallsign())
                .Times(1)
                .WillOnce(Return("BAW123"));

            EXPECT_CALL(mockFlightPlan, SetClearedAltitude(6000))
                .Times(1);

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

        TEST_F(InitialAltitudeEventHandlerTest, FlightPlanEventAcceptsDeprecatedSids)
        {
            ControllerPosition controller("LON_S_CTR", 129.420, "CTR", { "EGKK" });
            ActiveCallsign userCallsign(
                "LON_S_CTR",
                "Test",
                controller
            );
            callsigns.AddUserCallsign(userCallsign);

            airfields.AddAirfield(std::unique_ptr<Airfield>(new Airfield("EGKK", { "LON_S_CTR" })));
            owners.RefreshOwner("EGKK");

            EXPECT_CALL(mockFlightPlan, GetDistanceFromOrigin())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxDistanceFromOrigin));

            EXPECT_CALL(mockFlightPlan, HasControllerClearedAltitude())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsTracked())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, IsSimulated())
                .Times(1)
                .WillOnce(Return(false));

            EXPECT_CALL(mockFlightPlan, GetSidName())
                .Times(2)
                .WillRepeatedly(Return("#ADMAG2X"));

            EXPECT_CALL(mockFlightPlan, GetOrigin())
                .Times(4)
                .WillRepeatedly(Return("EGKK"));

            EXPECT_CALL(mockFlightPlan, GetCallsign())
                .Times(1)
                .WillOnce(Return("BAW123"));

            EXPECT_CALL(mockFlightPlan, SetClearedAltitude(6000))
                .Times(1);

            EXPECT_CALL(mockRadarTarget, GetGroundSpeed())
                .Times(1)
                .WillOnce(Return(handler.assignmentMaxSpeed));

            handler.FlightPlanEvent(mockFlightPlan, mockRadarTarget);
        }

    }  // namespace InitialAltitude
}  // namespace UKControllerPluginTest