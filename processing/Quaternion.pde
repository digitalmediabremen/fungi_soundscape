public class Quaternion {

   public float w,x,y,z;
   public PVector eulerAngles;

   public Quaternion(float x, float y, float z, float w){
     this.x = x;
     this.y = y;
     this.z = z;
     this.w = w;
     normalize();
   }

   public Quaternion(Quaternion q){
     this.w = q.w;
     this.x = q.x;
     this.y = q.y;
     this.z = q.z;
   }

   public Quaternion normalize() {
     float magnitude = w*w + x*x + y*y + z*z;
     if(magnitude != 0.0 && magnitude != 1.0){
       magnitude = 1.0f / sqrt(magnitude);
       w *= magnitude;
       x *= magnitude;
       y *= magnitude;
       z *= magnitude;
     }
     eulerAngles = setEulerAngles();
     return this;
   }

   public Quaternion set(float x, float y, float z, float w) {
     this.x = x;
     this.y = y;
     this.z = z;
     this.w = w;
     return normalize();
   }

   // Returns a rotation that rotates z degrees around 
   // the z axis, x degrees around the x axis, and y 
   // degrees around the y axis.
   public Quaternion Euler(){
      float roll = eulerAngles.x;
      float pitch = eulerAngles.y;
      float yaw = eulerAngles.z;

      float cr = (float)Math.cos(roll * 0.5);
      float sr = (float)Math.sin(roll * 0.5);
      float cp = (float)Math.cos(pitch * 0.5);
      float sp = (float)Math.sin(pitch * 0.5);
      float cy = (float)Math.cos(yaw * 0.5);
      float sy = (float)Math.sin(yaw * 0.5);

      w = cy*cr*cp + sy*sr*sp;
      x = cy*sr*cp - sy*cr*sp;
      y = cy*cr*sp + sy*sr*cp;
      z = sy*cr*cp - cy*sr*sp;
      return normalize();
   }


   // set euler angle representation of 
   // the rotation in 3-dim PVector
   private PVector setEulerAngles(){

     // roll: x-axis rotation
     float sinr = 2.0f * (w*x + y*z);
     float cosr = 1.0f - 2.0f * (x*x + y*y);
     float roll = (float)Math.atan2(sinr, cosr);

     // pitch: y-axis rotation
     float sinp = 2.0f * (w*y - z*x);
     float pitch = 0.0f;
     if(Math.abs(sinp) >= 1){
       pitch = (float)Math.copySign(Math.PI/2, sinp);
     } else {
       pitch = (float)Math.asin(sinp);
     }

     // yaw: z-axis rotation
     float siny = 2.0f * (w*z + x*y);
     float cosy = 1.0f - 2.0f * (y*y + z*z);
     float yaw = (float)Math.atan2(siny, cosy);

     return new PVector(roll, pitch, yaw);
   }
}
