//
// = AUTHOR(S)
//    Patric Jensfelt
//    
//    Copyright (c) 2004 Patric Jensfelt
//    

#include "SmartDataList.hh"

int main()
{
  Cure::SmartData sd, tmp;
  Cure::SmartDataList sdl;
  
  double t = 1;
  int index = 1;
  sd.setTime(t);t+=1;
  std::cerr << sdl.add(sd,index) << std::endl;index++;
  sd.setTime(t);t+=1;
  std::cerr << sdl.add(sd,index) << std::endl;index++;
  sd.setTime(t);t+=1;
  std::cerr << sdl.add(sd,index) << std::endl;index++;
  sd.setTime(t);t+=1;
  std::cerr << sdl.add(sd,index) << std::endl;index++;
  sd.setTime(t);t+=1;
  std::cerr << sdl.add(sd,index) << std::endl;index++;
  std::cerr << sdl.getLength() << std::endl;
  while (1) {
    sd.setTime(t);t+=1;
    std::cerr << sdl.add(sd,index) << std::endl;index++;
    sdl.remove(0);
    std::cerr << sdl.getLength() << std::endl;

    std::cerr << "===================================\n";
    int i = 0;
    while (1) {
      int ii;
      tmp.setTime(0);
      if (sdl.get(i, tmp, &ii) == 0) {
        std::cerr << tmp.getTime() << " " << ii << std::endl;
      } else {
        break;
      }
      i++;
    }

    //getchar();
    usleep(10000);
  }
}
