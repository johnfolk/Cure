#include "Pose3D.hh"
#include "PackedPose.hh"
#include "SensorPose.hh"
#include "PackedSensorPose.hh"
#include "FileAddress.hh"
#include "AddressBank.hh"
#include "SensorData.hh"

int main()
{
  Cure::Pose3D p;
  p.setX(1.234);
  p.setY(2.345);
  p.setTheta(3.456);
  p.setTime(Cure::Timestamp(123456789.123456));
  Cure::PackedPose pp(p);

  Cure::SensorPose sp(Cure::SensorData::SENSORTYPE_SICK, p, 42);
  Cure::PackedSensorPose psp(sp);

  Cure::FileAddress out;
  out.openWriteFile("tmp.tmp");

  out.write(p);
  out.write(pp);
  out.write(sp);
  out.write(psp);

  out.closeWriteFile();


  Cure::FileAddress in;
  in.openReadFile("tmp.tmp");
  Cure::SmartData sd;

  std::cerr << "=============================================\n";
  if (in.read(sd)) return -1;
  sd.print();

  std::cerr << "=============================================\n";
  if (in.read(sd)) return -1;
  sd.print();

  std::cerr << "=============================================\n";
  if (in.read(sd)) return -1;
  sd.print();

  std::cerr << "=============================================\n";
  if (in.read(sd)) return -1;
  sd.print();

  in.closeReadFile();
}
