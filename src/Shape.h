#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <string>
#include <vector>
#include <memory>



class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
    void loadMesh(const std::string &meshName, const std::string &mtlName);
	void init();
    void InitSegment(int i);
	void draw(const std::shared_ptr<Program> prog);
	void ComputeTex();
	void resize();
	
private:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
    std::vector<float> ambBuf;
    std::vector<float> specBuf;
    std::vector<float> difBuf;
    std::vector<float> shineBuf;
    std::vector< std::vector< float > > posBufs;
    std::vector< std::vector< float > > norBufs;
    std::vector< std::vector< float > > texBufs;
    std::vector< std::vector< unsigned int > > eleBufs;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
    int size;
    int numMaterials;
};

#endif
