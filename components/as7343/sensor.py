import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
    UNIT_MICROMOLE_PER_SQUARE_METER_SECOND,
)
from . import AS7343Component, CONF_AS7343_ID

CONF_F1 = "f1"
CONF_F2 = "f2"
CONF_FZ = "fz"
CONF_F3 = "f3"
CONF_F4 = "f4"
CONF_F5 = "f5"
CONF_FY = "fy"
CONF_FXL = "fxl"
CONF_F6 = "f6"
CONF_F7 = "f7"
CONF_F8 = "f8"
CONF_NIR = "nir"
CONF_CLEAR = "clear"

CONF_R_FR = "r_fr"
CONF_B_R = "b_r"
CONF_PAR_PROXY = "par_proxy"
CONF_PPFD = "ppfd"
CONF_PPFD_FACTOR = "ppfd_factor"

CHANNELS = [
    CONF_F1, CONF_F2, CONF_FZ, CONF_F3, CONF_F4, CONF_F5,
    CONF_FY, CONF_FXL, CONF_F6, CONF_F7, CONF_F8, CONF_NIR, CONF_CLEAR,
    CONF_R_FR, CONF_B_R, CONF_PAR_PROXY, CONF_PPFD
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_AS7343_ID): cv.use_id(AS7343Component),
        cv.Optional(CONF_PPFD_FACTOR, default=0.150): cv.float_,
        cv.Optional(CONF_F1): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F2): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_FZ): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F3): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F4): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F5): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_FY): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_FXL): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F6): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F7): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_F8): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_NIR): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_CLEAR): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_R_FR): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, accuracy_decimals=2, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_B_R): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, accuracy_decimals=2, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_PAR_PROXY): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_PPFD): sensor.sensor_schema(unit_of_measurement=UNIT_MICROMOLE_PER_SQUARE_METER_SECOND, accuracy_decimals=1, state_class=STATE_CLASS_MEASUREMENT),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_AS7343_ID])
    if CONF_PPFD_FACTOR in config:
        cg.add(parent.set_ppfd_factor(config[CONF_PPFD_FACTOR]))

    for channel in CHANNELS:
        if channel in config:
            sens = await sensor.new_sensor(config[channel])
            cg.add(getattr(parent, f"set_{channel}_sensor")(sens))
