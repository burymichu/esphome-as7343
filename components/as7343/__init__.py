import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

CODEOWNERS = ["@community"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

as7343_ns = cg.esphome_ns.namespace("as7343")
AS7343Component = as7343_ns.class_(
    "AS7343Component", cg.PollingComponent, i2c.I2CDevice
)

AS7343Gain = as7343_ns.enum("AS7343Gain")
GAINS = {
    "0.5X": AS7343Gain.AS7343_GAIN_0_5X,
    "1X": AS7343Gain.AS7343_GAIN_1X,
    "2X": AS7343Gain.AS7343_GAIN_2X,
    "4X": AS7343Gain.AS7343_GAIN_4X,
    "8X": AS7343Gain.AS7343_GAIN_8X,
    "16X": AS7343Gain.AS7343_GAIN_16X,
    "32X": AS7343Gain.AS7343_GAIN_32X,
    "64X": AS7343Gain.AS7343_GAIN_64X,
    "128X": AS7343Gain.AS7343_GAIN_128X,
    "256X": AS7343Gain.AS7343_GAIN_256X,
    "512X": AS7343Gain.AS7343_GAIN_512X,
}

CONF_AS7343_ID = "as7343_id"
CONF_GAIN = "gain"
CONF_ATIME = "atime"
CONF_ASTEP = "astep"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7343Component),
            cv.Optional(CONF_GAIN, default="1X"): cv.enum(GAINS, upper=True),
            cv.Optional(CONF_ATIME, default=29): cv.int_range(min=0, max=255),
            cv.Optional(CONF_ASTEP, default=599): cv.int_range(min=0, max=65535),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x39))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_atime(config[CONF_ATIME]))
    cg.add(var.set_astep(config[CONF_ASTEP]))
