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
	  pColor->A = 0;
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
	  pColor->A = 0;
	}

  std::ifstream ifm;
  ifm.open(InputName, std::ios::binary);
  ifm.read(data, XYZ*sizeof(char));
  ifm.close();

  ifm.open(GTName, std::ios::binary);
  ifm.read(data_gt, XYZ*sizeof(char));
  ifm.close();
  
  unsigned int XY = XSize*YSize;
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
	      
	      if(diff!=0 && data_gt[c]!=2 && data[c]!=2)
		{
		  // Check x direction
		  if(i==0       || (data_gt[c-1]-data[c-1])!=diff) 
		    IdxCount+=4;
		  if(i==XSize-1 || (data_gt[c+1]-data[c+1])!=diff) 
		    IdxCount+=4;	      
		  // Check y direction
		  if(j==0       || (data_gt[c-XSize]-data[c-XSize])!=diff) 
		    IdxCount+=4;
		  if(j==YSize-1 || (data_gt[c+XSize]-data[c+XSize])!=diff) 
		    IdxCount+=4;
		  // Check z direction
		  if(k==0       || (data_gt[c-XY]-data[c-XY])!=diff)
		    IdxCount+=4;
		  if(k==ZSize-1 || (data_gt[c+XY]-data[c+XY])!=diff)    
		    IdxCount+=4;		  
		}

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

	      if(diff!=0 && data_gt[c]!=2 && data[c]!=2)
		{
		  unsigned int offset = 0;
		  
		  if(diff==1)
		    offset = XYZ_plus;

		  if(i==0       || (data_gt[c-1]-data[c-1])!=diff) 
		    {
		      idx[ic++] = offset + kXY   + jX   + i;
		      idx[ic++] = offset + kXY_p + jX   + i;
		      idx[ic++] = offset + kXY_p + jX_p + i;	      	  
		      idx[ic++] = offset + kXY   + jX_p + i;
		    }

		  if(i==XSize-1 || (data_gt[c+1]-data[c+1])!=diff) 
		    {
		      idx[ic++] = offset + kXY   + jX   + i+1;
		      idx[ic++] = offset + kXY   + jX_p + i+1;
		      idx[ic++] = offset + kXY_p + jX_p + i+1;	 
		      idx[ic++] = offset + kXY_p + jX   + i+1;
		    }

		  if(j==0       || (data_gt[c-XSize]-data[c-XSize])!=diff) 
		    {
		      idx[ic++] = offset + kXY   + jX  + i;
		      idx[ic++] = offset + kXY   + jX  + i+1;
		      idx[ic++] = offset + kXY_p + jX  + i+1;
		      idx[ic++] = offset + kXY_p + jX  + i;
		    }

		  if(j==YSize-1 || (data_gt[c+XSize]-data[c+XSize])!=diff) 
		    {
		      idx[ic++] = offset + kXY_p + jX_p + i;
		      idx[ic++] = offset + kXY_p + jX_p + i+1;
		      idx[ic++] = offset + kXY   + jX_p + i+1;
		      idx[ic++] = offset + kXY   + jX_p + i; 
		    }
		  
		  if(k==0       || (data_gt[c-XY]-data[c-XY])!=diff)
		    {
		      idx[ic++] = offset + kXY   + jX   + i;		  		 
		      idx[ic++] = offset + kXY   + jX_p + i;
		      idx[ic++] = offset + kXY   + jX_p + i+1;
		      idx[ic++] = offset + kXY   + jX   + i+1;
		    }

		  if(k==ZSize-1 || (data_gt[c+XY]-data[c+XY])!=diff)    
		    {
		      idx[ic++] = offset + kXY_p + jX   + i;		       
		      idx[ic++] = offset + kXY_p + jX   + i+1;
		      idx[ic++] = offset + kXY_p + jX_p + i+1;
		      idx[ic++] = offset + kXY_p + jX_p + i;
		    }

		}
	           
	      c++;
	    }
	}
    }

  std::cout << "IdxCount: " << IdxCount << std::endl;

  // Clean up unsed vertices
  float*        cVertices = new float[IdxCount*4];
  unsigned int* cIdx      = new unsigned int[IdxCount];  

  unsigned int v=0;
  for(unsigned int i=0; i<IdxCount; i++)
    {
      cVertices[v++] = vertices[idx[i]*4  ];
      cVertices[v++] = vertices[idx[i]*4+1];
      cVertices[v++] = vertices[idx[i]*4+2];
      cVertices[v++] = vertices[idx[i]*4+3];

      cIdx[i] = i;
    }


  // Write results to output file
  std::ofstream ofm;
  ofm.open(OutputName, std::ios::trunc);

  ofm << "ply"                                               << std::endl;
  ofm << "format binary_little_endian 1.0"                   << std::endl;
  ofm << "element vertex "  << IdxCount                      << std::endl;
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
  ofm.write((char*)cVertices, IdxCount*4*sizeof(float));

  // Write faces
  unsigned char face_c = 4;
  int FaceSize = 4*sizeof(unsigned int);

  for(unsigned int i=0; i<IdxCount; i+=4)
    {
      ofm.write((char*)&face_c, 1);	  
      ofm.write((char*)&cIdx[i], FaceSize);
    }

  ofm.close();

  delete[] data;
  delete[] data_gt;
  delete[] vertices;
  delete[] idx;

  delete[] cVertices;
  delete[] cIdx;

  return 0;
}
