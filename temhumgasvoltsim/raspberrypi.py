## 아두이노에서 받은 ppm, volt, temperature, huminity 측정값 라즈베리파이에서 숫자값과 그래프로 나타내기

import serial
import time
import matplotlib.pyplot as plt

# 시리얼 포트 설정
ser = serial.Serial('/dev/ttyUSB0', 9600)  # 아두이노가 연결된 포트로 변경

# 데이터 저장을 위한 리스트
times = []
ppms = []
volts = []
temps = []
humis = []

# 그래프 설정
plt.ion()
fig, axs = plt.subplots(4, 1, figsize=(8, 6))
axs[0].set_title('Sensor Data')
axs[0].set_ylabel('PPM')
axs[1].set_ylabel('Voltage (V)')
axs[2].set_ylabel('Temperature (C)')
axs[3].set_ylabel('Humidity (%)')

while True:
    try:
        line = ser.readline().decode('utf-8').strip()  # 시리얼 데이터 읽기
        data = line.split(',')
        
        if len(data) == 5:
            # 데이터 파싱
            time_val = int(data[0])
            ppm = int(data[1])
            volt = float(data[2])
            temp = float(data[3])
            humi = float(data[4])

            # 데이터 저장
            times.append(time_val)
            ppms.append(ppm)
            volts.append(volt)
            temps.append(temp)
            humis.append(humi)

            # 데이터 수가 60을 초과하면 처음 30을 제거
            if len(times) > 60:
                times = times[30:]
                ppms = ppms[30:]
                volts = volts[30:]
                temps = temps[30:]
                humis = humis[30:]

            # 프롬프트 창에 데이터 출력
            print(f"Time: {time_val}s, PPM: {ppm}, Voltage: {volt:.2f}V, Temp: {temp:.1f}°C, Humidity: {humi:.1f}%")

            # 그래프 업데이트
            axs[0].clear()
            axs[0].plot(times, ppms, label='PPM', color='b')
            axs[1].clear()
            axs[1].plot(times, volts, label='Voltage', color='g')
            axs[2].clear()
            axs[2].plot(times, temps, label='Temperature', color='r')
            axs[3].clear()
            axs[3].plot(times, humis, label='Humidity', color='c')

            # 레이블 설정
            for ax in axs:
                ax.legend()
                ax.grid()
                ax.set_xlabel('Time (s)')
                
            plt.pause(1)  # 그래프 갱신 대기
    except KeyboardInterrupt:
        print("종료합니다.")
        break
    except Exception as e:
        print(f"Error: {e}")

ser.close()
