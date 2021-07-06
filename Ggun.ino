#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Mouse.h>

/* This driver reads raw data from the BNO055
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground
   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x29);

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  Serial.println("Orientation Sensor Raw Data Test"); Serial.println("");

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  delay(1000);

  /* Display the current temperature */
  int8_t temp = bno.getTemp();
  Serial.print("Current Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.println("");

  bno.setExtCrystalUse(true);

  Serial.println("Calibration status values: 0=uncalibrated, 3=fully calibrated");
}

/**************************************************************************/
/*
    Arduino loop function, called once 'setup' is complete (your own code
    should go here)
*/
/**************************************************************************/
void loop(void)
{
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  //imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  // Quaternion data
  imu::Quaternion quat = bno.getQuat();
  // 쿼터니언 정보를 회전행렬로 변환해서 사용할 예정입니다.

  /* Display calibration status for each sensor. */
  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);

  // 회전 행렬을 구하기 위한 정보들
  float x2 = quat.x() * quat.x();
  float y2 = quat.y() * quat.y();
  float z2 = quat.z() * quat.z();

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //쿼터니온에 대해 알지못하지만 방송에서 대각선움직임이 정상동작 안한다고 하여 검색해보니 인터넷에서는 w에 -가 없는것 같아 혹시 몰라 수정. 테스트 필요
  //https://enghqii.tistory.com/63
  float wx = quat.w() * quat.x();  //-quat.w() * quat.x();
  float wy = quat.w() * quat.y();  //-quat.w() * quat.y();
  float wz = quat.w() * quat.z();  //-quat.w() * quat.z();
  float xy = quat.x() * quat.y();
  float xz = quat.x() * quat.z();
  float yz = quat.y() * quat.z();

  // 회전 행렬, 열기반 벡터를 사용할 것
  float matrix[3][3] = {
      {0, 0, 0},  //{1.f - 2 * (y2 + z2), 2 * (xy - wz), 2 * (xz + wy)},
      {2 * (xy + wz), 1.f - 2 * (x2 + z2), 2 * (yz - wz)},
      {2 * (xz - wy), 2 * (yz + wx), 1.f - 2 * (x2 + y2)}
  };

  // 가속도 얻어오는 부분
  imu::Vector<3> acc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  float accX = acc.x();
  float accY = acc.y();
  float accZ = acc.z();

  // 좌표계가 변환된 가속도 얻어오는 부분, 이 부분이 진짜로 원하는 데이터 일 겁니다.
  //float newAccX = accX * matrix[0][0] + accY * matrix[0][1] + accZ * matrix[0][2];
  float newAccY = accX * matrix[1][0] + accY * matrix[1][1] + accZ * matrix[1][2];
  float newAccZ = accX * matrix[2][0] + accY * matrix[2][1] + accZ * matrix[2][2];

  // 변환된 가속도 테스트용 출력 부분
  //Serial.print("newAccX:");
  //Serial.print(newAccX);
  //Serial.print(" newAccY:");
  //Serial.print(newAccY);
  //Serial.print(" newAccZ:");
  //Serial.println(newAccZ);

  // 0.5보다 절대값이 작으면 0으로 변경
  if( fabs(newAccX) < 0.5 )
  {
    newAccX = 0;
  }
  if( fabs(newAccY) < 0.5 )
  {
    newAccX = 0;
  }

  Mouse.move(newAccY*-50, newAccZ*50, 0);  

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
