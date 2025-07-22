#include "base.h"
// #include <dirent.h>
#include <iostream>
#include <fstream>
#include <vector>


void savePointsToTxt(Pwn_vector& points, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

	unsigned long long pointsNum = points.size();
    for (unsigned long long i = 0; i < pointsNum; i++)
	{
		// std::cout<<"point: "<<vertex.x<<" "<<vertex.y<<" "<<vertex.z<<std::endl;
        outFile << points[i].first.x() << " " << points[i].first.y() << " " << points[i].first.z() << "\n";
	}	

    outFile.close();
    std::cout << "Saved " << points.size() << " points to " << filename << std::endl;
}


int main()
{

    const std::string wdir("/");

 
    // timer
    clock_t startp, endp;
    startp = clock();


    Pwn_vector res_points;  // wall points
    Pwn_vector floor_points;
    Pwn_vector ceiling_points;


    // string input_file = "";
    std::ifstream input_file("C:/SLAM/floorplan_code_v1/data/xyzc_normal.txt");

    float x, y, z, nx, ny, nz;
    int c;
    int num = 0;
    int tmp = 0;

    vector<Pwn_vector> input_points;    
    Pwn_vector tmp_points;
    double min_z = 1e9;
    double max_z = -1e9;
    double mean_z = 0;
    while (input_file >> x >> y >> z >> c >> nx >> ny >>nz) 
    {
        int l = c;
        if (z < min_z) min_z = z;
        if (z > max_z) max_z = z;
        mean_z += z;
        if(l == tmp)
        {
            tmp_points.push_back(make_pair(Point_3(x, y, z),Vector_3(nx, ny, nz)));
        }
        else
        {
            input_points.push_back(tmp_points);
            tmp = l;
            Pwn_vector().swap(tmp_points);
            tmp_points.push_back(make_pair(Point_3(x, y, z),Vector_3(nx, ny, nz)));
        }
        num += 1; 
    }
    mean_z /= num;
    std::cout << "Min z: " << min_z << ", Max z: " << max_z << ", Mean z: " << mean_z<<std::endl;
    std::cout<< num <<std::endl;

    for(int i = 0; i < input_points.size(); i++)
    {
        // std::cout<<input_points[i].size()<<std::endl;
        Pwn_vector fapoints = input_points[i];

        vector<Line> lines;     // facade projection lines
        vector<Line> floor_lines;
        vector<Line> ceiling_lines;

        vector<IntersectPoint> interpoints;     // segment intersection
        vector<Segment> segments;     // facade projection segments
        vector<Segment_2> selected;     // floorplan segments
        vector<Point_2> heights;    // segment z-height
        vector<Vector_2> oritations;    // segment normals
        vector<Segment> bsegments;    // boundary segments 
        unordered_map<Point_2, int> points_idx; // point index in pointcloud
        // obtain x and y boundaries
        float xmin = 1e9; float xmax = -1e9; float ymin = 1e9; float ymax = -1e9; float zmin = 1e9;
        for (int i = 0; i < fapoints.size(); i++)
        {
            if (fapoints[i].first.x() < xmin) xmin = fapoints[i].first.x();
            if (fapoints[i].first.x() > xmax) xmax = fapoints[i].first.x();
            if (fapoints[i].first.y() < ymin) ymin = fapoints[i].first.y();
            if (fapoints[i].first.y() > ymax) ymax = fapoints[i].first.y();
            if (fapoints[i].first.z() < zmin) zmin = fapoints[i].first.z();
        }
        // bbox egdes: a little bigger than boundaries
        xmin -= 2.0; xmax += 2.0; ymin -= 2.0; ymax += 2.0;
        bsegments.push_back(Segment(Segment_2(Point_2(xmin, ymin), Point_2(xmin, ymax))));
        bsegments.push_back(Segment(Segment_2(Point_2(xmin, ymin), Point_2(xmax, ymin))));
        bsegments.push_back(Segment(Segment_2(Point_2(xmax, ymin), Point_2(xmax, ymax))));
        bsegments.push_back(Segment(Segment_2(Point_2(xmax, ymax), Point_2(xmin, ymax))));

                //// Part 1. 2D facade detection
        if (1)
        {
            // detect 3D facade planes and then project them to generate 2D segments
            // load parameters
            const float probability = 0.001;
            const int min_points = 5000;
            const float epsilon = 0.02;
            const float cluster_epsilon = 0.50;
            const float normal_threshold = 0.90;
            const float cos_angle = 0.08;

            if (!ransac_detection_p(fapoints, probability, min_points, epsilon, cluster_epsilon, normal_threshold, cos_angle, lines, wdir, res_points, floor_points, ceiling_points, floor_lines, ceiling_lines, min_z, max_z, mean_z))
            {
                // std::cout<< "No facade plane is detected."<<std::endl;
            }
            else
            {
                std::cout << lines.size() << " facade projection lines are detected."<<std::endl;
            }
        }
    }

    // std::cout << res_points.size() << std::endl;
    savePointsToTxt(res_points, "C:/SLAM/floorplan_code_v1/data/wall.txt");
    savePointsToTxt(floor_points, "C:/SLAM/floorplan_code_v1/data/floor.txt");  
    savePointsToTxt(ceiling_points, "C:/SLAM/floorplan_code_v1/data/ceiling.txt");

    // PLYPointSave("../wall.ply", res_points);
    // PLYPointSave("../floor.ply", floor_points);
    // PLYPointSave("../ceiling.ply", ceiling_points);

    return 0;
}
