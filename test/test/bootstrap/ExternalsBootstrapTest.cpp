#include "pch/pch.h"
#include "bootstrap/ExternalsBootstrap.h"
#include "bootstrap/PersistenceContainer.h"
#include "curl/CurlRequest.h"

using UKControllerPlugin::Bootstrap::ExternalsBootstrap;
using UKControllerPlugin::Bootstrap::PersistenceContainer;
using UKControllerPlugin::Curl::CurlRequest;

namespace UKControllerPluginTest {
    namespace Bootstrap {

        class ExternalsBootstrapTest : public ::testing::Test {

            void SetUp() {
                Gdiplus::GdiplusStartupInput gdiStartup;
                Gdiplus::GdiplusStartup(&this->gdiPlusToken, &gdiStartup, NULL);
            }

            void TearDown() {
                Gdiplus::GdiplusShutdown(this->gdiPlusToken);
            }

            // Gdi plus token
            ULONG_PTR gdiPlusToken;
        };

        TEST_F(ExternalsBootstrapTest, BootstrapCreatesCurlApi)
        {
            PersistenceContainer container;
            HINSTANCE dll = 0;
            ExternalsBootstrap::Bootstrap(container, dll);

            EXPECT_NO_THROW(container.curl->MakeCurlRequest(CurlRequest("", CurlRequest::METHOD_GET)));
        }

        TEST_F(ExternalsBootstrapTest, BootstrapCreatesWindowsApi)
        {
            PersistenceContainer container;
            HINSTANCE dll = 0;
            ExternalsBootstrap::Bootstrap(container, dll);

            EXPECT_TRUE(
                container.windows->GetFullPathToLocalFile("testfile.json") == "ukcp/testfile.json"
            );
        }

        TEST_F(ExternalsBootstrapTest, BootstrapCreatesBrushes)
        {
            PersistenceContainer container;
            HINSTANCE dll = 0;
            ExternalsBootstrap::Bootstrap(container, dll);

            EXPECT_NO_THROW(container.brushes->euroscopeBackgroundBrush->GetType());
        }

        TEST_F(ExternalsBootstrapTest, BootstrapCreatesGraphicsWrapper)
        {
            PersistenceContainer container;
            HINSTANCE dll = 0;
            ExternalsBootstrap::Bootstrap(container, dll);

            HDC handle;
            EXPECT_NO_THROW(container.graphics->SetDeviceHandle(handle));
        }
    }  // namespace Bootstrap
}  // namespace UKControllerPluginTest
