# 아두이노와 라즈베리파이의 i2c통신을 통한 전류측정 코드

import time
import board
import busio
from adafruit_ina219 import INA219

# I2C 버스 초기화
i2c_bus = busio.I2C(board.SCL, board.SDA)

# INA219 객체 생성
ina219 = INA219(i2c_bus)

try:
    while True:
        # 측정값 출력
        bus_voltage = ina219.bus_voltage + ina219.shunt_voltage  # 전체 배터리 전압
        current = ina219.current  # mA 단위 전류
        power = ina219.power  # mW 단위 전력

        print(f"Voltage: {bus_voltage:.2f} V")
        print(f"Current: {current:.2f} mA")
        print(f"Power: {power:.2f} mW")
        print("--------------------")
        
        # 1초마다 측정 반복
        time.sleep(1)

except KeyboardInterrupt:
    print("측정을 종료합니다.")
