
import time
import smbus

BUS_NUMBER = 1

ACCEL_ADDR = 0x19
MAG_ADDR = 0x1E
GYRO_ADDR = 0x6B  # Try 0x6A if your board uses that address

bus = smbus.SMBus(BUS_NUMBER)


def read_u8(addr, reg):
    return bus.read_byte_data(addr, reg)


def write_u8(addr, reg, value):
    bus.write_byte_data(addr, reg, value)


def read_i16_le(addr, reg):
    """Read signed 16-bit little-endian register value."""
    low = read_u8(addr, reg)
    high = read_u8(addr, reg + 1)
    value = (high << 8) | low
    return value - 65536 if value >= 32768 else value


def read_i16_be(addr, reg):
    """Read signed 16-bit big-endian register value."""
    high = read_u8(addr, reg)
    low = read_u8(addr, reg + 1)
    value = (high << 8) | low
    return value - 65536 if value >= 32768 else value


def initialize():
    # LSM303DLHC accelerometer:
    # 100 Hz, all axes enabled
    write_u8(ACCEL_ADDR, 0x20, 0x57)

    # Block data update + high-resolution mode, +/-2 g
    write_u8(ACCEL_ADDR, 0x23, 0x88)

    # LSM303DLHC magnetometer:
    # 15 Hz output data rate
    write_u8(MAG_ADDR, 0x00, 0x14)

    # Gain setting: +/-1.3 gauss
    write_u8(MAG_ADDR, 0x01, 0x20)

    # Continuous-conversion mode
    write_u8(MAG_ADDR, 0x02, 0x00)

    # L3GD20 gyroscope:
    # Normal mode, 3 axes enabled, 95 Hz
    write_u8(GYRO_ADDR, 0x20, 0x0F)

    # +/-250 degrees/second
    write_u8(GYRO_ADDR, 0x23, 0x00)

    time.sleep(0.1)


def read_accelerometer():
    # LSM303DLHC output registers are little-endian.
    # In high-resolution mode, the 12-bit result is
    # left-justified in the 16-bit register.
    raw_x = read_i16_le(ACCEL_ADDR, 0x28) >> 4
    raw_y = read_i16_le(ACCEL_ADDR, 0x2A) >> 4
    raw_z = read_i16_le(ACCEL_ADDR, 0x2C) >> 4

    # +/-2 g range: approximately 1 mg per count
    return (
        raw_x * 0.001,
        raw_y * 0.001,
        raw_z * 0.001,
    )


def read_gyroscope():
    raw_x = read_i16_le(GYRO_ADDR, 0x28)
    raw_y = read_i16_le(GYRO_ADDR, 0x2A)
    raw_z = read_i16_le(GYRO_ADDR, 0x2C)

    # +/-250 dps sensitivity: 8.75 millidegrees/s per count
    scale = 0.00875

    return (
        raw_x * scale,
        raw_y * scale,
        raw_z * scale,
    )


def read_magnetometer():
    # LSM303DLHC magnetometer register order is X, Z, Y.
    # The values are big-endian.
    raw_x = read_i16_be(MAG_ADDR, 0x03)
    raw_z = read_i16_be(MAG_ADDR, 0x05)
    raw_y = read_i16_be(MAG_ADDR, 0x07)

    # Gain setting +/-1.3 gauss:
    # X/Y = 1100 counts/gauss, Z = 980 counts/gauss
    return (
        raw_x / 1100.0,
        raw_y / 1100.0,
        raw_z / 980.0,
    )


try:
    # Verify device identity before configuring.
    accel_id = read_u8(ACCEL_ADDR, 0x0F)
    gyro_id = read_u8(GYRO_ADDR, 0x0F)

    mag_id = (
        read_u8(MAG_ADDR, 0x0A),
        read_u8(MAG_ADDR, 0x0B),
        read_u8(MAG_ADDR, 0x0C),
    )

    print(f"Accelerometer ID: 0x{accel_id:02X}")
    print(f"Magnetometer ID: {[hex(x) for x in mag_id]}")
    print(f"Gyroscope ID: 0x{gyro_id:02X}")

    if accel_id != 0x33:
        print("WARNING: unexpected accelerometer ID")

    if mag_id != (0x48, 0x34, 0x33):
        print("WARNING: unexpected magnetometer ID")

    if gyro_id not in (0xD4, 0xD7):
        print("WARNING: unexpected gyroscope ID")

    initialize()

    print("\nReading sensors. Press Ctrl+C to stop.\n")

    while True:
        ax, ay, az = read_accelerometer()
        gx, gy, gz = read_gyroscope()
        mx, my, mz = read_magnetometer()

        print(
            f"ACC [g] "
            f"X={ax:+7.3f} Y={ay:+7.3f} Z={az:+7.3f} | "
            f"GYRO [deg/s] "
            f"X={gx:+8.2f} Y={gy:+8.2f} Z={gz:+8.2f} | "
            f"MAG [gauss] "
            f"X={mx:+7.3f} Y={my:+7.3f} Z={mz:+7.3f}"
        )

        time.sleep(0.2)

except KeyboardInterrupt:
    print("\nTest stopped.")

except OSError as error:
    print(f"I2C communication error: {error}")

finally:
    bus.close()
