__kernel void marchingsquare(__global float* datain,__global int* dataout,float threshold,int linesize)
{
  int idy = get_global_id(0);
  int idx = get_global_id(1);

  if(idy < linesize && idx < linesize){
	  int topleftid = idy*linesize+idx;
	  int toprightid = idy*linesize+idx+1;
	  int bottomleftid = (idy+1)*linesize+idx;
	  int bottomrightid = (idy+1)*linesize+idx+1;

	  int lookupid = 0;

	  if(datain[topleftid] > threshold){
		lookupid =lookupid + 1;
	  }
	  if(datain[toprightid] > threshold){
		lookupid =lookupid + 2;
	  }
	  if(datain[bottomleftid] > threshold){
		lookupid =lookupid + 4;
	  }
	  if(datain[bottomrightid] > threshold){
		lookupid =lookupid + 8;
	  }

	  int outputid = idy*(linesize-1)+idx;

	  dataout[outputid] = lookupid;
  }
}