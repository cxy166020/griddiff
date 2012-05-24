#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

struct RGBA
{
  unsigned char R,G,B,A;
};

int main(int argc, char** argv)
{
  int ArgCount = 1;
  char* InputName  = argv[ArgCount++];
  char* GTName     = argv[ArgCount++];
  char* OutputName = argv[ArgCount++];

  int XSize = atoi(argv[ArgCount++]);
  int YSize = atoi(argv[ArgCount++]);
  int ZSize = atoi(argv[ArgCount++]);

  int XYZ      = XSize*YSize*ZSize;
  int XYZ_plus = (XSize+1)*(YSize+1)*(ZSize+1);

  int XYZ_plux3 = XYZ_plus*3;

  char* data    = new char[XYZ];
  char* data_gt = new char[XYZ];

  // Ever vertex has two color candidates
  float* vertices = new float[XYZ_plus*8];

  unsigned int c = 0;
 
  // Red vertices
  for(int k=0; k<=ZSize; k++)
    for(int j=0; j<=YSize; j++)
      for(int i=0; i<=XSize; i++)
	{
	  vertices[c++] = (float)i;
	  vertices[c++] = (float)j;
	  vertices[c++] = (float)k;

	  RGBA* pColor = (RGBA*)&vertices[c++];
	  pColor->R = 255;
	  pColor->G = 0;
	  pColor->B = 0;
	  pColor->A = 125;
	}

  // Green vertices
  for(int k=0; k<=ZSize; k++)
    for(int j=0; j<=YSize; j++)
      for(int i=0; i<=XSize; i++)
	{
	  vertices[c++] = (float)i;
	  vertices[c++] = (float)j;
	  vertices[c++] = (float)k;

	  RGBA* pColor = (RGBA*)&vertices[c++];
	  pColor->R = 0;
	  pColor->G = 255;
	  pColor->B = 0;
	  pColor->A = 125;
	}

  std::ifstream ifm;
  ifm.open(InputName, std::ios::binary);
  ifm.read(data, XYZ*sizeof(char));
  ifm.close();

  ifm.open(GTName, std::ios::binary);
  ifm.read(data_gt, XYZ*sizeof(char));
  ifm.close();
  
  // Count number of visible faces
  unsigned int IdxCount =0;
  c = 0;
  
  for(int k=0; k<ZSize; k++)
    {
      for(int j=0; j<YSize; j++)
	{
	  for(int i=0; i<XSize; i++)
	    {
	      // If input and ground truth have the same value
	      // skip it
	      char diff = data_gt[c] - data[c];
	      
	      if(diff!=0)
		IdxCount+=24;

	      c++;
	    }
	}
    }

  unsigned int* idx = new unsigned int[IdxCount];  
  unsigned int ic = 0;
  c = 0;

  for(int k=0; k<ZSize; k++)
    {
      unsigned int kXY   = k*(XSize+1)*(YSize+1);
      unsigned int kXY_p = (k+1)*(XSize+1)*(YSize+1);

      for(int j=0; j<YSize; j++)
	{
	  unsigned int jX   = j*(XSize+1);
	  unsigned int jX_p = (j+1)*(XSize+1);

	  for(int i=0; i<XSize; i++)
	    {
	      char diff = data_gt[c] - data[c];

	      if(diff!=0)
		{
		  unsigned int offset = 0;
		  
		  // if(diff==1)
		  //   offset = XYZ_plux3;

		  idx[ic++] = offset + kXY   + jX   + i;
		  idx[ic++] = offset + kXY_p + jX   + i;
		  idx[ic++] = offset + kXY_p + jX_p + i;	      	  
		  idx[ic++] = offset + kXY   + jX_p + i;

		  idx[ic++] = offset + kXY   + jX   + i+1;
		  idx[ic++] = offset + kXY   + jX_p + i+1;
		  idx[ic++] = offset + kXY_p + jX_p + i+1;	 
		  idx[ic++] = offset + kXY_p + jX   + i+1;

		  idx[ic++] = offset + kXY   + jX  + i;
		  idx[ic++] = offset + kXY   + jX  + i+1;
		  idx[ic++] = offset + kXY_p + jX  + i+1;
		  idx[ic++] = offset + kXY_p + jX  + i;

		  idx[ic++] = offset + kXY_p + jX_p + i;
		  idx[ic++] = offset + kXY_p + jX_p + i+1;
		  idx[ic++] = offset + kXY   + jX_p + i+1;
		  idx[ic++] = offset + kXY   + jX_p + i;	      	  	      	 	      	  

		  idx[ic++] = offset + kXY   + jX   + i;		  		 
		  idx[ic++] = offset + kXY   + jX_p + i;
		  idx[ic++] = offset + kXY   + jX_p + i+1;
		  idx[ic++] = offset + kXY   + jX   + i+1;

		  idx[ic++] = offset + kXY_p + jX   + i;		       
		  idx[ic++] = offset + kXY_p + jX   + i+1;
		  idx[ic++] = offset + kXY_p + jX_p + i+1;
		  idx[ic++] = offset + kXY_p + jX_p + i;

		}
	           
	      c++;
	    }
	}
    }

  std::cout << "IdxCount: " << IdxCount << std::endl;


  // Write results to output file
  std::ofstream ofm;
  ofm.open(OutputName, std::ios::trunc);

  ofm << "ply"                                               << std::endl;
  ofm << "format binary_little_endian 1.0"                   << std::endl;
  ofm << "element vertex "  << XYZ_plus*2                    << std::endl;
  ofm << "property float x"                                  << std::endl;
  ofm << "property float y"                                  << std::endl;
  ofm << "property float z"                                  << std::endl;
  ofm << "property uchar red"                                << std::endl;
  ofm << "property uchar green"                              << std::endl;
  ofm << "property uchar blue"                               << std::endl;
  ofm << "property uchar alpha"                              << std::endl;
  ofm << "element face " << (int)(IdxCount/4)                << std::endl;
  ofm << "property list uchar uint vertex_indices"           << std::endl;
  ofm << "end_header"                                        << std::endl;

  // Write vertex
  ofm.write((char*)vertices, XYZ_plus*8*sizeof(float));

  // Write faces
  unsigned char face_c = 4;
  int FaceSize = 4*sizeof(unsigned int);

  for(unsigned int i=0; i<IdxCount; i+=4)
    {
      ofm.write((char*)&face_c, 1);	  
      ofm.write((char*)&idx[i], FaceSize);
    }

  ofm.close();

  delete[] data;
  delete[] data_gt;
  delete[] vertices;
  delete[] idx;

  return 0;
}
