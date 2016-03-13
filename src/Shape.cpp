#include "Shape.h"
#include <iostream>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>

#include "GLSL.h"
#include "Program.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0)
{
}

Shape::~Shape()
{
}

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
        //shapes = objShapes;
        //materials = objMaterials;
        //cout << "Number of shapes: " << shapes.size() << " in " << meshName << endl;
        size = shapes.size();
        numMaterials = objMaterials.size();
        
        for(int i=0; i < shapes.size(); i++) {
            posBuf = shapes[i].mesh.positions;
            norBuf = shapes[i].mesh.normals;
            texBuf = shapes[i].mesh.texcoords;
            eleBuf = shapes[i].mesh.indices;
            
            posBufs.push_back(posBuf);
            norBufs.push_back(norBuf);
            texBufs.push_back(texBuf);
            eleBufs.push_back(eleBuf);
            
            if(objMaterials.size() > 0) {
                ambBuf.push_back(objMaterials[i].ambient[0]);
                ambBuf.push_back(objMaterials[i].ambient[1]);
                ambBuf.push_back(objMaterials[i].ambient[2]);
                
                specBuf.push_back(objMaterials[i].specular[0]);
                specBuf.push_back(objMaterials[i].specular[1]);
                specBuf.push_back(objMaterials[i].specular[2]);
                
                difBuf.push_back(objMaterials[i].diffuse[0]);
                difBuf.push_back(objMaterials[i].diffuse[1]);
                difBuf.push_back(objMaterials[i].diffuse[2]);
                
                shineBuf.push_back(objMaterials[i].shininess);
            }
            
        }
        
	}
}

/* Note this is fairly dorky - */
void Shape::ComputeTex() {
	float u, v;

		for (size_t n = 0; n < norBuf.size()/3; n++) {
			u = norBuf[n*3+0]/2.0 + 0.5;
			v = norBuf[n*3+1]/2.0 + 0.5;
         texBuf[n*3+0] = u;
         texBuf[n*3+1] = v;
      }
}


void Shape::resize() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

    for(int i=0; i < this->size; i++) {
        //Go through all vertices to determine min and max of each dimension
        for (size_t v = 0; v < posBufs[i].size() / 3; v++) {
            if(posBufs[i][3*v+0] < minX) minX = posBufs[i][3*v+0];
            if(posBufs[i][3*v+0] > maxX) maxX = posBufs[i][3*v+0];
            
            if(posBufs[i][3*v+1] < minY) minY = posBufs[i][3*v+1];
            if(posBufs[i][3*v+1] > maxY) maxY = posBufs[i][3*v+1];
            
            if(posBufs[i][3*v+2] < minZ) minZ = posBufs[i][3*v+2];
            if(posBufs[i][3*v+2] > maxZ) maxZ = posBufs[i][3*v+2];
        }
        
        //From min and max compute necessary scale and shift for each dimension
        float maxExtent, xExtent, yExtent, zExtent;
        xExtent = maxX-minX;
        yExtent = maxY-minY;
        zExtent = maxZ-minZ;
        if (xExtent >= yExtent && xExtent >= zExtent) {
            maxExtent = xExtent;
        }
        if (yExtent >= xExtent && yExtent >= zExtent) {
            maxExtent = yExtent;
        }
        if (zExtent >= xExtent && zExtent >= yExtent) {
            maxExtent = zExtent;
        }
        scaleX = 2.0 /maxExtent;
        shiftX = minX + (xExtent/ 2.0);
        scaleY = 2.0 / maxExtent;
        shiftY = minY + (yExtent / 2.0);
        scaleZ = 2.0/ maxExtent;
        shiftZ = minZ + (zExtent)/2.0;
        
        //Go through all verticies shift and scale them
        for (size_t v = 0; v < posBufs[i].size() / 3; v++) {
            posBufs[i][3*v+0] = (posBufs[i][3*v+0] - shiftX) * scaleX;
            
            assert(posBufs[i][3*v+0] >= -1.0 - epsilon);
            assert(posBufs[i][3*v+0] <= 1.0 + epsilon);
            posBufs[i][3*v+1] = (posBufs[i][3*v+1] - shiftY) * scaleY;
            assert(posBufs[i][3*v+1] >= -1.0 - epsilon);
            assert(posBufs[i][3*v+1] <= 1.0 + epsilon);
            posBufs[i][3*v+2] = (posBufs[i][3*v+2] - shiftZ) * scaleZ;
            assert(posBufs[i][3*v+2] >= -1.0 - epsilon);
            assert(posBufs[i][3*v+2] <= 1.0 + epsilon);
            
        }
    }
   
}
void Shape::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		norBufID = 0;
	} else {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		//texBufID = 0;
		//send in spherical constructed
		for (size_t v = 0; v < posBuf.size(); v++) {
         texBuf.push_back(0);
      }
      ComputeTex();

		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::InitSegment(int i)
{
    //cout << "Calling InitSegment on shape " << i << endl;
    // Send the position array to the GPU
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
    glBufferData(GL_ARRAY_BUFFER, posBufs[i].size()*sizeof(float), &posBufs[i][0], GL_STATIC_DRAW);
    
    // Send the normal array to the GPU
    if(norBufs[i].empty()) {
        norBufID = 0;
    } else {
        glGenBuffers(1, &norBufID);
        glBindBuffer(GL_ARRAY_BUFFER, norBufID);
        glBufferData(GL_ARRAY_BUFFER, norBufs[i].size()*sizeof(float), &norBufs[i][0], GL_STATIC_DRAW);
    }
    
    // Send the texture array to the GPU
    if(texBufs[i].empty()) {
        //texBufID = 0;
        //send in spherical constructed
        for (size_t v = 0; v < posBufs[i].size(); v++) {
            texBufs[i].push_back(0);
        }
        ComputeTex();
        
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBufs[i].size()*sizeof(float), &texBufs[i][0], GL_STATIC_DRAW);
    } else {
        glGenBuffers(1, &texBufID);
        glBindBuffer(GL_ARRAY_BUFFER, texBufID);
        glBufferData(GL_ARRAY_BUFFER, texBufs[i].size()*sizeof(float), &texBufs[i][0], GL_STATIC_DRAW);
    }
    
    // Send the element array to the GPU
    glGenBuffers(1, &eleBufID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBufs[i].size()*sizeof(unsigned int), &eleBufs[i][0], GL_STATIC_DRAW);
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog)
{
    //Somehow make it so that drawing goes through the shape[] array and changes the buffs and stuff
    
    for(int i=0; i < this->size; i++ ) {
        InitSegment(i);
        // Bind position buffer
        int h_pos = prog->getAttribute("vertPos");
        GLSL::enableVertexAttribArray(h_pos);
        glBindBuffer(GL_ARRAY_BUFFER, posBufID);
        glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        
        // Bind normal buffer
        int h_nor = prog->getAttribute("vertNor");
        if(h_nor != -1 && norBufID != 0) {
            GLSL::enableVertexAttribArray(h_nor);
            glBindBuffer(GL_ARRAY_BUFFER, norBufID);
            glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        }
        
        // Bind texcoords buffer
        int h_tex = prog->getAttribute("vertTex");
        if(h_tex != -1 && texBufID != 0) {
            GLSL::enableVertexAttribArray(h_tex);
            glBindBuffer(GL_ARRAY_BUFFER, texBufID);
            glVertexAttribPointer(h_tex, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
        }
        
        if(numMaterials > 0) {
            //Pass in ambient color
            glUniform3f(prog->getUniform("MatAmb"), ambBuf[i*3], ambBuf[i*3 + 1],  ambBuf[i*3 + 2]);
            
            //Pass in specular color
            glUniform3f(prog->getUniform("Spec"), specBuf[i*3], specBuf[i*3 + 1],  specBuf[i*3 + 2]);
            
            
            //Pass in diffuse color
            glUniform3f(prog->getUniform("MatDif"), difBuf[i*3], difBuf[i*3 + 1],  difBuf[i*3 + 2]);
            
            
            //Pass in shininess
            glVertexAttrib1f(prog->getUniform("Shine"), shineBuf[i]);
        }
        
        
        
        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
        
        
        
        // Draw
        glDrawElements(GL_TRIANGLES, (int)eleBufs[i].size(), GL_UNSIGNED_INT, (const void *)0);
        
        // Disable and unbind
        if(h_tex != -1) {
            GLSL::disableVertexAttribArray(h_tex);
        }
        if(h_nor != -1) {
            GLSL::disableVertexAttribArray(h_nor);
        }
        GLSL::disableVertexAttribArray(h_pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    
	
}
