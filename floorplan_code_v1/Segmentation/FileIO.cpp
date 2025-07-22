#include "ply.h"
#include "base.h"

// generate polygon mesh
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
// #include <boost/function_output_iterator.hpp>


typedef struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	uint8_t r, g, b;
} Vertex;

typedef struct Face
{
	unsigned char nverts; 		// number of vertex indices in list 
	int *verts; 		// vertex index list 
} Face;

char *elem_names[] = {(char*)"vertex", (char*)"face"};

PlyProperty vert_props[] = 
{
	// list of property information for a vertex
	{(char*)"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, x), 0, 0, 0, 0},
	{(char*)"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, y), 0, 0, 0, 0},
	{(char*)"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, z), 0, 0, 0, 0},
	{(char*)"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, nx), 0, 0, 0, 0},
	{(char*)"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, ny), 0, 0, 0, 0},
	{(char*)"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex, nz), 0, 0, 0, 0},
	{(char*)"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, r), 0, 0, 0, 0},
	{(char*)"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, g), 0, 0, 0, 0},
	{(char*)"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, b), 0, 0, 0, 0},
};

PlyProperty face_props[] = 
{
	// list of property information for a vertex 
	{(char*)"vertex_indices", PLY_INT, PLY_INT, offsetof(Face, verts), 1, PLY_UCHAR, PLY_UCHAR, offsetof(Face, nverts)},
};



bool PLYPointSave(const string fileName, Pwn_vector& points, int type)
{
	if (points.empty() || fileName.empty())
		return false;

	unsigned long long pointsNum = points.size();
	PlyFile *ply;
	float version;
		
	ply = ply_open_for_writing(fileName.c_str(), 1, elem_names, type, &version);

	ply_element_count(ply, (char*)"vertex", pointsNum);
	ply_describe_property(ply, (char*)"vertex", &vert_props[0]);
	ply_describe_property(ply, (char*)"vertex", &vert_props[1]);
	ply_describe_property(ply, (char*)"vertex", &vert_props[2]);
	ply_describe_property(ply, (char*)"vertex", &vert_props[3]);
	ply_describe_property(ply, (char*)"vertex", &vert_props[4]);
	ply_describe_property(ply, (char*)"vertex", &vert_props[5]);

	ply_header_complete(ply);
	
	Vertex vertex;
	ply_put_element_setup(ply, (char*)"vertex");
	ply->file_type = 1;
	// std::cout<< ply->file_type << std::endl;
	for (unsigned long long i = 0; i < pointsNum; i++)
	{
		vertex.x = points[i].first.x();
		vertex.y = points[i].first.y();
		vertex.z = points[i].first.z();
		// std::cout<<"point: "<<vertex.x<<" "<<vertex.y<<" "<<vertex.z<<std::endl;
		vertex.nx = points[i].second.x();
		vertex.ny = points[i].second.y();
		vertex.nz = points[i].second.z();

		ply_put_element(ply, (void *)&vertex);
	}	
	
	ply_close(ply);
	return true;
}





