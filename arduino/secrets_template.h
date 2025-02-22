#include <pgmspace.h>

#define SECRET
#define THINGNAME "esp32"

const char WIFI_SSID[] = "XXXXXXXXX";
const char WIFI_PASSWORD[] = "XXXXXXXXXXXXXXX";
const char AWS_IOT_ENDPOINT[] = "XXXXXXXXXXXXXX-ats.iot.ap-northeast-1.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
AAAAAAAAAAAAAAAAAAAAAa
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
AAAAAAAAAAAAAAAAAAAAAa
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
AAAAAAAAAAAAAAAAAAAAAa
-----END RSA PRIVATE KEY-----
)KEY";
