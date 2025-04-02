# Source of truth

set(PEX87XX_PROJECT		"libpex87xx")
set(PEX87XX_DESCRIPTION		"Linux I2C Interface to PEX87xx PCIe Switches")
set(PEX87XX_HOMEPAGE_URL	"https://github.com/benmcollins/pex87xx")

set(PEX87XX_VERSION_SET		0 0 0)

set(PEX87XX_SO_CRA		0 0 0)
# SONAME History
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html

string(TOLOWER ${PEX87XX_PROJECT} PEX87XX_PROJECT_LOWER)
list(GET PEX87XX_VERSION_SET 0 DEFINE_MAJOR)
list(GET PEX87XX_VERSION_SET 1 DEFINE_MINOR)
list(GET PEX87XX_VERSION_SET 2 DEFINE_MICRO)
string(JOIN "." PEX87XX_VERSION ${PEX87XX_VERSION_SET})
set(DEFINE_VERSION "\"${PEX87XX_VERSION}\"")

list(GET PEX87XX_SO_CRA 0 PEX87XX_SO_CURRENT)
list(GET PEX87XX_SO_CRA 1 PEX87XX_SO_REVISION)
list(GET PEX87XX_SO_CRA 2 PEX87XX_SO_AGE)

# Libtool does -version-info cur:rev:age, but cmake does things
# a bit different. However, the result is the same.
math(EXPR PEX87XX_SO_MAJOR "${PEX87XX_SO_CURRENT} - ${PEX87XX_SO_AGE}")
set(PEX87XX_VERSION_INFO "${PEX87XX_SO_MAJOR}.${PEX87XX_SO_AGE}.${PEX87XX_SO_REVISION}")
set(PEX87XX_COMPATVERSION "${PEX87XX_SO_MAJOR}")
