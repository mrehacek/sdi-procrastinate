import processing.serial.*;
import processing.video.*;

Serial ardSerial;

Movie irritatingMovie;
Movie blackScreenMovie;
Boolean isIrritatingMovie;

void setup() {
  printArray(Serial.list());
  ardSerial = new Serial(this, Serial.list()[0], 9600);
  
  fullScreen();
  background(0);
  isIrritatingMovie = true;
  //getBoolean();
  
  irritatingMovie = new Movie(this, "video_irritante.mp4");
  blackScreenMovie = new Movie (this, "black_vid.mp4");
  
  
    if(isIrritatingMovie){
    irritatingMovie.play();
    irritatingMovie.jump(random(irritatingMovie.duration()));
    }
    
    else{
      blackScreenMovie.loop();
    }
}

void draw() {
  String valStr = ardSerial.readStringUntil('\n');
  if (valStr != null) {
    isIrritatingMovie = parseBoolean(valStr.split(" ")[0]);
  }
  isIrritatingMovie = true;
  
  //getBoolean();
  
  if(isIrritatingMovie){
    
    blackScreenMovie.stop();
    irritatingMovie.play();
    
    
    image(irritatingMovie, 0, 0);
  //  if(irritatingMovie.time()==irritatingMovie.duration()-2.0){
  //    irritatingMovie.jump(1.0);
  //  }
  }
  else{
    irritatingMovie.stop();
    blackScreenMovie.loop();
    
    image(blackScreenMovie, 0, 0);
  }
}

// Called every time a new frame is available to read
void movieEvent(Movie m) {
  m.read();
}


//Boolean getBoolean(){
  
//}
