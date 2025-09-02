#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <tt_sdk/sdk.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include "../include/tt_sdk/plugins/logging.h"

#define BUFFER_SIZE 8192

static char* enrollmentCaCert = NULL;
static char* enrollmentClientCert = NULL;
static char* enrollmentPrivKey = NULL;
static TtEnrollmentData enrollmentData;
/* Populate here your device private key content */
#define ENROLLMENT_CA "-----BEGIN CERTIFICATE-----\nMIIFpTCCA42gAwIBAgIUGyv6YxXp6XFX7BDBwMOAOhlZ44cwDQYJKoZIhvcNAQEL\nBQAwWjELMAkGA1UEBhMCQVQxFjAUBgNVBAgMDVVwcGVyIEF1c3RyaWExDTALBgNV\nBAcMBExpbnoxDDAKBgNVBAoMA0lvVDEWMBQGA1UEAwwNZW5yb2xsbWVudC1jYTAe\nFw0yNDAxMTUxMDExMjFaFw0yOTAxMTQxMDExMjFaMFoxCzAJBgNVBAYTAkFUMRYw\nFAYDVQQIDA1VcHBlciBBdXN0cmlhMQ0wCwYDVQQHDARMaW56MQwwCgYDVQQKDANJ\nb1QxFjAUBgNVBAMMDWVucm9sbG1lbnQtY2EwggIiMA0GCSqGSIb3DQEBAQUAA4IC\nDwAwggIKAoICAQC5zEywb1wG2I4sdimRpNe9UvxMv8IrnzewqdtjLmMtjemsuCnl\n2csHgk/ykIbatAOFkUwPl/VXXXkurF6vO5TC9qWj5HRFCjpxkDmS7wzKKzdWsMro\nFSp6ExahsGaNVkddfzpbpzI3PNCTKC8KPd8B7uZ+r6EziqWg+zwjKXiVT6jZIHvN\nNj18ojnsIDdtwbisDd4/7KGFh3mjAaC/iL3TJJUX1E3q0ptks1pPeE/nU409abuc\nM02pjldQ8RW9EuqN3ZBIrWCi5Ms/cw9ZXWXvSHPkOBE2p0RjJ4fuNJI6PIdnFelk\ntN6kwDBCh0R9N+t/O5JQad9uMa5SiL66pZGDxwVzrN9gHQ8dXPubAfwnLIAqfqxC\ngP/wuKvj8ZEOCue4UG03KFDfnGcBxzaTkjXtH5rO1ZWxk6XSPVVVBkrXQljLmgzX\nc0TbuSQr6f3UR5b4cNtKCs0VtkFNP7NfD8Vpd0igV6Ry5vhBHgaA+M3ISDRexw6M\nd71XhVR2bTRaA0nNTPbnE50McdTspbfrAJQednr8oYsppd21NeZB72bNQ+UuhHq+\nAG0EQX5kAlJSjpGoYWf/GwXs1XI2EAtjY6263p7yBHFiH0tYvfVl51Ffe9e3xQcB\n5ZIUawO+qjTzpC+3B3QYkMxWNSXvAvjl3OI7qSCo5O+viuK31jN0jeGVjwIDAQAB\no2MwYTASBgNVHRMBAf8ECDAGAQH/AgEAMAsGA1UdDwQEAwIBBjAdBgNVHQ4EFgQU\naFp5odk+nVtn/AsSvCQVIWhYP5gwHwYDVR0jBBgwFoAUaFp5odk+nVtn/AsSvCQV\nIWhYP5gwDQYJKoZIhvcNAQELBQADggIBAFMhr0wtInyoa7rDwBCuEn5K+xkNf6WE\ns4Gn+thA4aag00xbLmqlzzgSsDYRqxa30fv3zBJ30qDRci1XkgUGDoKOHchLnV7H\nTCL9m17MlqhqPWtz+tmytqDZVBcsRK39+qTi0zpDlQBOyKGoV+Ont4PkrkPhEwpb\nHHR4YLEtlUvt4n6/2CGDkVBOyQZFr8t6sLdNANlLXJWrSzmZl7IvVEcRhYv0MH9B\nAnNhn3sHahupY5HreEnjQb88fYgpWwa+cOXzB0HCFEWLOqa++pH5e0j1aELoRYsw\nTNh+2q5Y9BcrylQKu2V/4sf1wzqQvBrdFlyRHo1m5tAV3kAnM/jC67fZHj8qUvl5\ngwCaKwuRJsOOZoWE9Ttz5wofEfcwFJllrpcJ+orZnIe8ZXQocVcvuRhPXSKRyJP4\nneIUqqStdKjJBUuEXxuiOm63HOWGULLAMqTdncv1IMwLn6jVx55tWj7ZbUxeYDY3\n2wjXfkijQ4jWAR5wk+97v8C9RG/ZyZdOBkpEclzBX0/nBVS3b9Bbden3pglvFAoq\naYrRUnAICdWFfWlYwK9aaMMH662wsu/QdQ+rq2GjpPfhM76cRo/fSS4eyQOy8Tns\nR1gaXjUAz0ahq/KYXJKs4yDRoNDf3745ZmYJgKLHFtYTBb3b3etUAosl7zCpQM0Y\nA4LOR1Nc/90p\n-----END CERTIFICATE-----";

#define ENROLLMENT_CRT "-----BEGIN CERTIFICATE-----\nMIIE4zCCApugAwIBAgIUOalzD80s/k6Ua2BAakzdhgEAAAAwPQYJKoZIhvcNAQEK\nMDCgDTALBglghkgBZQMEAgGhGjAYBgkqhkiG9w0BAQgwCwYJYIZIAWUDBAIBogMC\nASAwWjELMAkGA1UEBhMCQVQxFjAUBgNVBAgMDVVwcGVyIEF1c3RyaWExDTALBgNV\nBAcMBExpbnoxDDAKBgNVBAoMA0lvVDEWMBQGA1UEAwwNZW5yb2xsbWVudC1jYTAe\nFw0yNTA0MjQwODA3MzBaFw0yNzA0MjQwODA3MzBaMGYxCzAJBgNVBAYTAkFUMRYw\nFAYDVQQIDA1VcHBlciBBdXN0cmlhMQ0wCwYDVQQHDARMaW56MQ8wDQYDVQQKDAZj\ndXN0b20xHzAdBgNVBAMMFmVucm9sbG1lbnQtY2VydC1jdXN0b20wggEiMA0GCSqG\nSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC8djlRKImbohc9H86CURYEHhNKvM46/mAo\nMz7kpQeiDfOyvNLEiTq4CZzs7VqTdBanD7mrBQJeCpiQIFPkdMdkYCwP5oMxDkrX\npSWcKpWUvKX39q7A/gzrSCOYMwoDsYzrHtZ0ffFjIPu9279I6kupwFSVauSQrKcj\nyTkXhmn6FP+XPnvNFW/FthXnPZ28FyUjeNlrMv0TRuaDEjfbSrG8J4ksneoVjP0U\npQ3NraPwkf19bK8wJr5LAb+QnonFn95vHdg/PkQAEKwhTl+hxLtxprAz3fTT1VZL\nMxBvP5dftmAZFpPKOxL4O7ktCH7jVMfClKompQ76aLqM896hp9MtAgMBAAGjNTAz\nMA8GA1UdEwEB/wQFMAMCAQAwCwYDVR0PBAQDAgWgMBMGA1UdJQQMMAoGCCsGAQUF\nBwMCMD0GCSqGSIb3DQEBCjAwoA0wCwYJYIZIAWUDBAIBoRowGAYJKoZIhvcNAQEI\nMAsGCWCGSAFlAwQCAaIDAgEgA4ICAQBM1yQwd3Y73UXgfbyVFQRo2r6p2IrF9nnG\n7iaYPuoQw2lyao/5nrxs5vCdS2qMnNF17aFY0E81xqdYMtJELDJIynVCKp5Ak4e9\nPWHmRHJdyAHdHZ6mOZkVAoVSuD3FM9yew9i4SdKhk3VV3Y0SsM67ktvna+Soc0vs\nsw09gYZr2q219GWVBSlD6209LfpBdr+woTtS9+gUV/h01RgZJeczA3ZY4NUIAT6c\n/Dv4nV1Pom9mD/KWqwgAO8Voz4Tpoqho5XdbfAlbXZfALVeCxXYZcj0cmCU9kCgF\nacQSAH0R2PkUVH8tQHCCskLzC6luzfxrjYris6smnHDw3vCN2C7osGPJaSSpVBAs\n++oYXyahC+MtlbgAwoJu3GPb2tWYqlF+rTsKI1E6jrFgdoKDJn/tUKz/yMtMiWam\n/ggosO54ILLPfo2Zg9ciw5HtcrmnPHF7U66jQLVx8Bu2d8WFYfifD03Ph1UbDu26\nvtJaU+DoFw+qycKtkJt03HA7zb4FdvrajRJWDfLl9KrLvmN3AJCj6NikXYtB8vwo\nZmMs/g1viGAoOYwROkr56Kg5zgrQzn/HAFuzZ4qFM7s1HLRYNyJWebVQ0gqr2KEd\nVjPV4YawKpnIFJtf2xlZs2xBHZOIOl9ifktiQg6nmP3MrjARy28HW5801UGc/9sG\nWnNbzkZMUg==\n-----END CERTIFICATE-----";  

#define ENROLLMENT_KEY "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC8djlRKImbohc9\nH86CURYEHhNKvM46/mAoMz7kpQeiDfOyvNLEiTq4CZzs7VqTdBanD7mrBQJeCpiQ\nIFPkdMdkYCwP5oMxDkrXpSWcKpWUvKX39q7A/gzrSCOYMwoDsYzrHtZ0ffFjIPu9\n279I6kupwFSVauSQrKcjyTkXhmn6FP+XPnvNFW/FthXnPZ28FyUjeNlrMv0TRuaD\nEjfbSrG8J4ksneoVjP0UpQ3NraPwkf19bK8wJr5LAb+QnonFn95vHdg/PkQAEKwh\nTl+hxLtxprAz3fTT1VZLMxBvP5dftmAZFpPKOxL4O7ktCH7jVMfClKompQ76aLqM\n896hp9MtAgMBAAECggEAFMYTW1bUlEsxtJMk+Ehn3NxT1BErymTvxH6hmCJ4zOws\nyK73rKJTzqWAnr0rFsXdGV0qYieTMzqHxptJpA0bTX0sXjRSkb25kjuqAaTQCC0j\nPkn6yVs3oVe6VLSLjgysM96aVQzs5rhkB3GJjEgHPxbsO+wkda4SXmd325f9Zotp\nEch9QCw/B2q2bf0aAOJMoLZkrsBaZBHfIdntc9fK59i6iuLn7bEMqMsShVxgS0J+\nqwk6DX7k5RxHMekQ4i/EWk3jagK6PzvnS1Cv2+eKu6bbkqbEn/ZbIhXvbjuhE7x9\nrkh656YL70TT/UuUBQXmfCIfRD5pv1DYUK36THjeIQKBgQD98+5+ip3JjpNJD1n7\ntGPXzBVUo+kr2QoxNo5I68d9IKUQPt+6NjKM0rEi1QZe2HN3lfEjWfVHHt0OZAXJ\nVPUtwRTLNtqT5bNlS0t2Jqff/szewG+rhwTffjcFL37sxajKuB96RkfrACSLrmvO\nmPwLp8dxOZ5Lq5rb3UGsUoN7QwKBgQC9+yRdV61CSzPi7nGqm0BKHbs2Wa3aJIpG\nCQFDx+uT4EVvcfGvJH4TfO4hY0nMVhtWFJAPtY6m8SJFgjRLPVSO3rfEdeLUCbQU\nZpjFUFNWX8zN/tRfLKxZlaoPEgAx2n5dtu5E4uNvyiL4HxaL+y6nDb051ZQET0eB\nE9T2HtW4zwKBgEaSFuCLXLW6LSvMXFEaG/TH/dV7hzxsH+z1IC0beGBJRhVxo3J3\nxSxlEFyBnjeVJuFAhbYxNXwCE6QAH6uHNGr2kPS31Z3r4yJu/hVjQJt6jywHhgDA\nkJYa2OdT/42EkBdleznbdfjUHVPDsKnE0aXbhgzs2hRlJ6+ZmLpuY6NJAoGBAKmz\nZWdPwz9dh6qlDbm01neeGTXSY55hV3Z54rxWH5PHGJ8VLCsjsM7doKUIWMqC/AYn\ndgviRNvQt6lZRjcTQK4iOgrutsBrLoWuo7ZVKywThRFZGQEvGYEoVHEDxlLB9nDl\nz+6OsVSRPi1fL7e4lpK4jhfUNVSoMJoXSfrTzls3AoGAdHfUBFqbxWdJtNiYasiI\n/fPXs4Nv6UVihe1h8x18Jk4VfARwqrvuGUVR5nFRHvE7boeiCHauwFvpN3GcV1V4\n1osLm4baXspV3zaBjV5Fge1BoiyOP9oYhkOCUrBVOB1faD1aewoU/FYs94btmLQG\nFJK0apP7YjXOG3OFO5OEKqk=\n-----END PRIVATE KEY-----\n";

const char preloadedRSAjson[] = "{\n"
"  \"public\": \"-----BEGIN PUBLIC KEY-----\\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAr01wi1M6LCCqozqx3Wa4\\n"
"M93tHijcB2JyrtlxeZy0AxdZ/Kn/XhDBg6vjrS1HD29XqDsbYFfEa930J3yt0Pcq\\n"
"7AT7FhdVeEmS6JxKxHi/vBG7yznOe11X9gow0mTUEm4aN53jAPZ6LrkPlZ0QMu/J\\n"
"NLPQieMtXd4oCQXBXIFXg9WlOcOWh/mERySOwvzuhnC0I2DKbPkrtdV7y33uXRsx\\n"
"3TGwWpUpv/MAn7f4TtQeWzA7jJkMkcr24BGpOI1ZGgkgTqH63RtlDdGB0rcdFU9O\\n"
"qWZuGI2aEHSsUlU7/+2RCWRe/9SbsGHTicv/F6yGVovGa1YMwOPyH2bc/v9LvLt9\\n"
"GwIDAQAB\\n"
"-----END PUBLIC KEY-----\\n\",\n"
"  \"private\": \"-----BEGIN PRIVATE KEY-----\\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCvTXCLUzosIKqj\\n"
"OrHdZrgz3e0eKNwHYnKu2XF5nLQDF1n8qf9eEMGDq+OtLUcPb1eoOxtgV8Rr3fQn\\n"
"fK3Q9yrsBPsWF1V4SZLonErEeL+8EbvLOc57XVf2CjDSZNQSbho3neMA9nouuQ+V\\n"
"nRAy78k0s9CJ4y1d3igJBcFcgVeD1aU5w5aH+YRHJI7C/O6GcLQjYMps+Su11XvL\\n"
"fe5dGzHdMbBalSm/8wCft/hO1B5bMDuMmQyRyvbgEak4jVkaCSBOofrdG2UN0YHS\\n"
"tx0VT06pZm4YjZoQdKxSVTv/7ZEJZF7/1JuwYdOJy/8XrIZWi8ZrVgzA4/IfZtz+\\n"
"/0u8u30bAgMBAAECggEAGcFCWi3Zm5cXCtE6RYFsaa5ewnIVVX5ow/iNW+Uia1vY\\n"
"bC8bphnHHI257vz6djGRCtXYQVDTuldLIiwGq29M3o0iyI2YEIqaq1MfBvuIi9x9\\n"
"Hy/4fpZpsGux3Y9TuvtzHZ20NrV/oPKub/g544noITaT2lZGmYZkkmLGlE0qauNP\\n"
"in5C29q5T+Fk6twzOpnZy87ALthO1b671EBMDNCK06X+w+ORU0igJuxs2Ct8c7LB\\n"
"bgoV0mORPR8r3+Xuxd2FyZWl/mUokus5M7nCY8pa1vtOktyVmlnjo++2agIejbZy\\n"
"uDmtBbpXzUHNEUmLOV0v3X1OQ76hhlamiNDuF75waQKBgQDqHFbdQ9c0vhDmg5U2\\n"
"iwThHwyoEXCSHOcSDKqBxWg7ZjLDeyHuLnAJnw0rmy0JkjuOPt0CRJazvoOJNapX\\n"
"ijiwSfSq/XAdVKNg7Rq7/UhLA5GlTWDCAWQkzqlfBcDQrwdi+fOLmcQXkVnB3WIg\\n"
"iDqraUbEoMZYgwq9PPIcpKjOowKBgQC/sXhwhN96XZLiiNZcdYlaSoTeqaKjTO2u\\n"
"ECC4UyPgicV/gd2hkBqOeLw/UaeffQCciED0vVdC923mm0QjzIGTBKbxMZGLR95g\\n"
"GOm05aaCtwQph1UV4IXf4iDQNZInxTuJ9PxE0Hs1lIN1msmvp+L5FzgPgWqNE1Jv\\n"
"qxgMJzNXKQKBgGcrH/iYWfFrRSVgnVQRBZBzz28NaG9rf0UMAeP4a8upaPuOetBs\\n"
"9IC47+PkmX9bSxWPjHPgaA1ECtFfrfav+fVuUf08fLjgQLOJKbvojYBJTVjsdZRp\\n"
"aolx7V/ruCH+0CGFBxfhJnJAYq61cfXmMvyxAzfJpj8BWK02e3e19JJDAoGAY5bP\\n"
"+6UMdYYnyYlN1Ls7oK0WWpfG5xAQa131oc2P1he9g4D0o/s50MrpAfxqGXHX33rq\\n"
"RDwaGbByaFxCClc0+ixwjt4xvyqXXVWUFWc6Gq76epjUm4kKBEZ2xbUPOZo+VG3y\\n"
"oc2uNSfzJCZGofuW79IgTe3ubVycd2dsFuRGyAECgYEAkMBBNI1iK7xkkATfb6V4\\n"
"rC2meg4Q9S1BOi3MZM1Duaa7eT4q8cnbDncpjjl9e1Q7V2pilDfSDTV/06Vgv/nY\\n"
"ZkA5/1Yk20R8fG0r9yz60kbKlTOAnZPCVBOgu3yGXDPIM4q0KfZe6QrTaHX91faf\\n"
"zbT3sBBhXJItzJCATNCb00A=\\n"
"-----END PRIVATE KEY-----\\n\"\n"
"}";

//	"clientCertificate": "-----BEGIN CERTIFICATE-----\nMIIE5DCCApygAwIBAgIVALJbV0CvFhBBsCoAm4YMtEsBAAAAMD0GCSqGSIb3DQEB\nCjAwoA0wCwYJYIZIAWUDBAIBoRowGAYJKoZIhvcNAQEIMAsGCWCGSAFlAwQCAaID\nAgEgMFoxCzAJBgNVBAYTAkFUMRYwFAYDVQQIDA1VcHBlciBBdXN0cmlhMQ0wCwYD\nVQQHDARMaW56MQwwCgYDVQQKDANJb1QxFjAUBgNVBAMMDWVucm9sbG1lbnQtY2Ew\nHhcNMjUwNTAyMTgxNzIyWhcNMjcwNTAyMTgxNzIyWjBmMQswCQYDVQQGEwJBVDEW\nMBQGA1UECAwNVXBwZXIgQXVzdHJpYTENMAsGA1UEBwwETGluejEPMA0GA1UECgwG\nY3VzdG9tMR8wHQYDVQQDDBZlbnJvbGxtZW50LWNlcnQtY3VzdG9tMIIBIjANBgkq\nhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvHY5USiJm6IXPR/OglEWBB4TSrzOOv5g\nKDM+5KUHog3zsrzSxIk6uAmc7O1ak3QWpw+5qwUCXgqYkCBT5HTHZGAsD+aDMQ5K\n16UlnCqVlLyl9/auwP4M60gjmDMKA7GM6x7WdH3xYyD7vdu/SOpLqcBUlWrkkKyn\nI8k5F4Zp+hT/lz57zRVvxbYV5z2dvBclI3jZazL9E0bmgxI320qxvCeJLJ3qFYz9\nFKUNza2j8JH9fWyvMCa+SwG/kJ6JxZ/ebx3YPz5EABCsIU5focS7caawM93009VW\nSzMQbz+XX7ZgGRaTyjsS+Du5LQh+41THwpSqJqUO+mi6jPPeoafTLQIDAQABozUw\nMzAPBgNVHRMBAf8EBTADAgEAMAsGA1UdDwQEAwIFoDATBgNVHSUEDDAKBggrBgEF\nBQcDAjA9BgkqhkiG9w0BAQowMKANMAsGCWCGSAFlAwQCAaEaMBgGCSqGSIb3DQEB\nCDALBglghkgBZQMEAgGiAwIBIAOCAgEAaJkWEpixLwEaN3rPeQmjMUq4lZWCBx2g\nrPcLHZCsbQdaQDQjk2WXP1mH05+nwGa2AYn2HAIE+6ubM4T4e0K5wSLTclVAzZIr\nJFJpoZMmruYTGMt546jGxEnFHmn9VGeod9myB6s/MpmVlwCLLdF7CYKZCYxhpuIM\nbOwtRqY1QBMO+mAxNAqjspCY9VXx3YaofxjpzrUK00rEcNjsxHUklNrtc7LOILbk\nZ73US/jtjtC2Ixqg3oGaJyn7ofK5r3vUCTuoVCtoYndkmMcUv/EaDjqJgVFmY2+A\nfr1LP7HtERATQCaJOSrBJOavIEVplFhEOWnyJ4rjyvFuETmV1w5hw5QIX/vf5T7/\nPq+rv0fEBGrDjyAtGg/pOPBi/DSViX2HZysRy39FiVg9B3p/pyvrQk0QTBX0H64E\ndemai61aMRLdLQtT6q16awhelhL1K6/hfuOqxHXT7E6Knt3N1jZVlsIQSc+endaw\nG7Tb4SMX0lKl4y9tcGpQvk4U9qMS0252pAp94EbcG282PSoCr9lG30AHHOECvHsC\nin6k37kLfX9C0YvWKuL0TS9PsfchrUmBwkw6eSBmWqq4l0D1/VR2Y+OwlsVm4sna\ni4HwmIDTYa0zTNenMa+grPgCJ9ASmGUBHGfHcG0k7W3K7oqdIzTh3yXVSNkFL2eG\nZ76aMc4qKV0=\n-----END CERTIFICATE-----",
//	"enrollmentCaCertificate": "-----BEGIN CERTIFICATE-----\nMIIFpTCCA42gAwIBAgIUGyv6YxXp6XFX7BDBwMOAOhlZ44cwDQYJKoZIhvcNAQEL\nBQAwWjELMAkGA1UEBhMCQVQxFjAUBgNVBAgMDVVwcGVyIEF1c3RyaWExDTALBgNV\nBAcMBExpbnoxDDAKBgNVBAoMA0lvVDEWMBQGA1UEAwwNZW5yb2xsbWVudC1jYTAe\nFw0yNDAxMTUxMDExMjFaFw0yOTAxMTQxMDExMjFaMFoxCzAJBgNVBAYTAkFUMRYw\nFAYDVQQIDA1VcHBlciBBdXN0cmlhMQ0wCwYDVQQHDARMaW56MQwwCgYDVQQKDANJ\nb1QxFjAUBgNVBAMMDWVucm9sbG1lbnQtY2EwggIiMA0GCSqGSIb3DQEBAQUAA4IC\nDwAwggIKAoICAQC5zEywb1wG2I4sdimRpNe9UvxMv8IrnzewqdtjLmMtjemsuCnl\n2csHgk/ykIbatAOFkUwPl/VXXXkurF6vO5TC9qWj5HRFCjpxkDmS7wzKKzdWsMro\nFSp6ExahsGaNVkddfzpbpzI3PNCTKC8KPd8B7uZ+r6EziqWg+zwjKXiVT6jZIHvN\nNj18ojnsIDdtwbisDd4/7KGFh3mjAaC/iL3TJJUX1E3q0ptks1pPeE/nU409abuc\nM02pjldQ8RW9EuqN3ZBIrWCi5Ms/cw9ZXWXvSHPkOBE2p0RjJ4fuNJI6PIdnFelk\ntN6kwDBCh0R9N+t/O5JQad9uMa5SiL66pZGDxwVzrN9gHQ8dXPubAfwnLIAqfqxC\ngP/wuKvj8ZEOCue4UG03KFDfnGcBxzaTkjXtH5rO1ZWxk6XSPVVVBkrXQljLmgzX\nc0TbuSQr6f3UR5b4cNtKCs0VtkFNP7NfD8Vpd0igV6Ry5vhBHgaA+M3ISDRexw6M\nd71XhVR2bTRaA0nNTPbnE50McdTspbfrAJQednr8oYsppd21NeZB72bNQ+UuhHq+\nAG0EQX5kAlJSjpGoYWf/GwXs1XI2EAtjY6263p7yBHFiH0tYvfVl51Ffe9e3xQcB\n5ZIUawO+qjTzpC+3B3QYkMxWNSXvAvjl3OI7qSCo5O+viuK31jN0jeGVjwIDAQAB\no2MwYTASBgNVHRMBAf8ECDAGAQH/AgEAMAsGA1UdDwQEAwIBBjAdBgNVHQ4EFgQU\naFp5odk+nVtn/AsSvCQVIWhYP5gwHwYDVR0jBBgwFoAUaFp5odk+nVtn/AsSvCQV\nIWhYP5gwDQYJKoZIhvcNAQELBQADggIBAFMhr0wtInyoa7rDwBCuEn5K+xkNf6WE\ns4Gn+thA4aag00xbLmqlzzgSsDYRqxa30fv3zBJ30qDRci1XkgUGDoKOHchLnV7H\nTCL9m17MlqhqPWtz+tmytqDZVBcsRK39+qTi0zpDlQBOyKGoV+Ont4PkrkPhEwpb\nHHR4YLEtlUvt4n6/2CGDkVBOyQZFr8t6sLdNANlLXJWrSzmZl7IvVEcRhYv0MH9B\nAnNhn3sHahupY5HreEnjQb88fYgpWwa+cOXzB0HCFEWLOqa++pH5e0j1aELoRYsw\nTNh+2q5Y9BcrylQKu2V/4sf1wzqQvBrdFlyRHo1m5tAV3kAnM/jC67fZHj8qUvl5\ngwCaKwuRJsOOZoWE9Ttz5wofEfcwFJllrpcJ+orZnIe8ZXQocVcvuRhPXSKRyJP4\nneIUqqStdKjJBUuEXxuiOm63HOWGULLAMqTdncv1IMwLn6jVx55tWj7ZbUxeYDY3\n2wjXfkijQ4jWAR5wk+97v8C9RG/ZyZdOBkpEclzBX0/nBVS3b9Bbden3pglvFAoq\naYrRUnAICdWFfWlYwK9aaMMH662wsu/QdQ+rq2GjpPfhM76cRo/fSS4eyQOy8Tns\nR1gaXjUAz0ahq/KYXJKs4yDRoNDf3745ZmYJgKLHFtYTBb3b3etUAosl7zCpQM0Y\nA4LOR1Nc/90p\n-----END CERTIFICATE-----"

const char tt_ca_cert[] = ENROLLMENT_CA;
const char tt_cli_cert[] = ENROLLMENT_CRT;
const char tt_cli_pk[] = ENROLLMENT_KEY;

static const TtEnrollmentData enrollmentData_const = {
	// fill with your enrollment data!
	.server = "dev-y.tributech-node.com",
	.caCert = tt_ca_cert,
	.clientCert = tt_cli_cert,
	.privKey = tt_cli_pk
};


TtState oldState_local = TT_SDK_STATE_NOT_INITIALIZED; 
TtState newState_local = TT_SDK_STATE_NOT_INITIALIZED;
char log_msg[128];
extern void zephyrLog(void*, TtLogLevel level, const char* msg);



static char* readFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Could not open '%s'\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, size, file);
    buffer[size] = ' ';
    fclose(file);
    return buffer;
}

static const TtEnrollmentData* loadEnrollmentData(const char* folder, const char* server) {
    char path[BUFFER_SIZE];

    snprintf(path, BUFFER_SIZE, "%s/enrollment.ca", folder);
    enrollmentCaCert = readFile(path);

    snprintf(path, BUFFER_SIZE, "%s/enrollment.crt", folder);
    enrollmentClientCert = readFile(path);

    snprintf(path, BUFFER_SIZE, "%s/enrollment.key", folder);
    enrollmentPrivKey = readFile(path);

    enrollmentData.server = server;
    enrollmentData.caCert = enrollmentCaCert;
    enrollmentData.clientCert = enrollmentClientCert;
    enrollmentData.privKey = enrollmentPrivKey;

    return &enrollmentData;
}

static const char* loadPreLoadedRsaKeys(const char* folder) {
    char path[BUFFER_SIZE];
    snprintf(path, BUFFER_SIZE, "%s/pre-loaded-rsa.json", folder);


    if (fs_stat(path, NULL) != 0) {
        printf("Pre-loaded RSA key '%s' not found!\n", path);
        printf("RSA key will be generated by the SDK...\n");
        return NULL;
    }

    char* fileContent = readFile(path);
    if (!fileContent) return NULL;

    size_t bufferSize = strlen(fileContent) + 1;
    char* buffer = malloc(bufferSize);
    strncpy(buffer, fileContent, bufferSize);
    free(fileContent);

    printf("Pre-loaded RSA key '%s' loaded\n", path);
    return buffer;
}

static void sdkHeartbeat() {
    printf("Tributech SDK heartbeat received\n");
}

static void sdkStateChanged(TtState oldState, TtState newState) {
    newState_local = newState;
	oldState_local = oldState;
    printf("Tributech SDK state changed from '%s' to '%s'\n", ttStateToString(oldState), ttStateToString(newState));
}

void sendTestDataLoop() {
    TtError err;
    TtStreamId streamInt32 = { 0 };
    if (TT_IS_ERR(ttGetStreamByName("Int32 Stream", &streamInt32))) {
        printf("Could not find int32 stream\n");
    }

    TtStreamId streamInt64 = { 0 };
    if (TT_IS_ERR(ttGetStreamByName("Int64 Stream", &streamInt64))) {
        printf("Could not find int64 stream\n");
    }

    TtStreamId streamFloat = { 0 };
    if (TT_IS_ERR(ttGetStreamByName("Float Stream", &streamFloat))) {
        printf("Could not find float stream\n");
    }

    TtStreamId streamDouble = { 0 };
    if (TT_IS_ERR(ttGetStreamByName("Double Stream", &streamDouble))) {
        printf("Could not find double stream\n");
    }

    TtStreamId streamUtf8 = { 0 };
    if (TT_IS_ERR(ttGetStreamByName("UTF8 Stream", &streamUtf8))) {
        printf("Could not find utf8 stream\n");
    }

    for (uint16_t val = 0;; ++val) {
        if (streamInt32.id[0] != ' ') {
            err = ttAddInt32(&streamInt32, (int32_t)val);
            if (TT_IS_ERR(err)) {
                printf("Could not add value to int32 stream: %s\n", ttErrorToString(err));
            }
        }

        if (streamInt64.id[0] != ' ') {
            err = ttAddInt64(&streamInt64, (int64_t)val);
            if (TT_IS_ERR(err)) {
                printf("Could not add value to int64 stream: %s\n", ttErrorToString(err));
            }
        }

        if (streamFloat.id[0] != ' ') {
            err = ttAddFloat(&streamFloat, (float)val);
            if (TT_IS_ERR(err)) {
                printf("Could not add value to float stream: %s\n", ttErrorToString(err));
            }
        }

        if (streamDouble.id[0] != ' ') {
            err = ttAddDouble(&streamDouble, (double)val);
            if (TT_IS_ERR(err)) {
                printf("Could not add value to double stream: %s\n", ttErrorToString(err));
            }
        }

        if (streamUtf8.id[0] != ' ') {
            char utf8[32];
            snprintf(utf8, sizeof(utf8), "%u - %s", val, ((val & 1) != 0) ? "ð" : "ð");
            err = ttAddUtf8(&streamUtf8, utf8);
            if (TT_IS_ERR(err)) {
                printf("Could not add value to utf8 stream: %s\n", ttErrorToString(err));
            }
        }

        k_sleep(K_SECONDS(1));
    }
}

int main(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    // Connect to the Tributech MQTT broker
TtConfig config = {
	.enrollment = &enrollmentData_const,
	.heartbeatCallback = sdkHeartbeat,
	.stateCallback = sdkStateChanged,
	.mqttBroker= "demeter-mqtt.dev-y.tributech-node.com",
	.preLoadedRsaKey = 	preloadedRSAjson,
};


    TtError err = ttInit(&config);

	if (TT_IS_ERR(err)) {
		// something went wrong
		snprintf(log_msg, sizeof(log_msg), "ttInit failed with error code: %d", (int)err);
		zephyrLog(NULL, TT_LL_ERROR, log_msg);
		// Log the error string
		snprintf(log_msg, sizeof(log_msg), "ttInit error: %s", ttErrorToString(err));
		zephyrLog(NULL, TT_LL_ERROR, log_msg);
	}
	else {
		// everything is fine
		int32_t data = 0;
		TtStreamId streamInt32 = { 0 };
		do{ // main work loop
			if(newState_local == TT_SDK_STATE_RUNNING) {
				if(data % 100 == 0) {
					// send data up to the tributech cloud
					if(ttGetStreamByName("Int32 Stream", &streamInt32) == TT_E_OK) { // if stream exist write data to it
						if(ttAddInt32(&streamInt32, data) != TT_E_OK) {
							printf("Failed to add data!");
						}
					}

			    }
				data++;
			}

		} while (true); // (newState_local != TT_SDK_STATE_RUNNING);
	}
    k_sleep(K_SECONDS(3));
}


int main_for_linux(int argc, char* argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <enrollment-folder> <enrollment-server> <mqtt-server>\n", argv[0]);
        return 1;
    }

    const char* enrollmentFolder = argv[1];
    const char* enrollmentServer = argv[2];
    const char* mqttServer = argv[3];

    struct stat st;
    if (stat(enrollmentFolder, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Enrollment folder '%s' does not exist!", enrollmentFolder);
        return 1;
    }

    if (strchr(enrollmentServer, '/') || !strchr(enrollmentServer, '.')) {
        fprintf(stderr, "Invalid enrollment server '%s'!", enrollmentServer);
        return 1;
    }

    if (strchr(mqttServer, '/') || !strchr(mqttServer, '.')) {
        fprintf(stderr, "Invalid MQTT server '%s'!", mqttServer);
        return 1;
    }

    TtConfig config = {
        .enrollment = loadEnrollmentData(enrollmentFolder, enrollmentServer),
        .heartbeatCallback = sdkHeartbeat,
        .stateCallback = sdkStateChanged,
        .mqttBroker = mqttServer,
        .preLoadedRsaKey = loadPreLoadedRsaKeys(enrollmentFolder),
    };

    TtError err = ttInit(&config);
    if (TT_IS_ERR(err)) {
        printf("Could not initialize the Tributech SDK: %s", ttErrorToString(err));
        return 1;
    }

    sendTestDataLoop();
    return 0;
}
