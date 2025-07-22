#include "base.h"
#include <limits>

#include <CGAL/property_map.h>
#include <CGAL/Shape_detection_3.h>
#include <CGAL/Regularization/regularize_planes.h>

// Type declarations
typedef CGAL::First_of_pair_property_map<Point_with_normal>     Point_map;
typedef CGAL::Second_of_pair_property_map<Point_with_normal>    Normal_map;
typedef CGAL::Shape_detection_3::Efficient_RANSAC_traits<K, Pwn_vector, Point_map, Normal_map>    Traits;
typedef CGAL::Shape_detection_3::Efficient_RANSAC<Traits>     Efficient_ransac;
typedef CGAL::Shape_detection_3::Plane<Traits>    Plane;
typedef CGAL::Shape_detection_3::Cylinder<Traits>     Cylinder;

/*!
 * \description: Project facade plane to line/segment.
 * \param plane detected facade plane by RANSAC
 * \param points facade pointcloud
 * \param num plane/line index
 * \param lines projection line set
 * \return
 */
void PtoS(Plane* plane, Pwn_vector& points, int num, vector<Line>& lines)
{
    Line line;
    line.ID = num;
    double sum_distances = 0;
    int N = 0;
    double xmin = 1e9; double xmax = -1e9; 
    double x1, y1, x2 ,y2;
    // compute line parameter
    double k = -plane->plane_normal().x()/plane->plane_normal().y();
    double b = 0;
    Vector_2 normal(plane->plane_normal().x(), plane->plane_normal().y());
    Line_2 dir(Point_2(0,0), normal.perpendicular(CGAL::POSITIVE));
    

    vector<size_t>::const_iterator index_it = plane->indices_of_assigned_points().begin();
    double vx = 0, vy = 0;
    double disx = 0; 
    while (index_it != plane->indices_of_assigned_points().end()) 
    {
        // retrieves point
        const Point_with_normal &p = *(points.begin() + (*index_it));  
        sum_distances += CGAL::sqrt(plane->squared_distance(p.first));
        Point_2 p2(Point_2(p.first.x(),p.first.y()));
        line.pointset.push_back(p.first);
        if(line.height0 < p.first.z()) // max
            line.height0 = p.first.z();
        if(line.height1 > p.first.z()) // min
            line.height1 = p.first.z();
        b += -k*p.first.x() + p.first.y();
        vx += p.second.x();
        vy += p.second.y();
        if(plane->plane_normal().y() == 0) disx += p.first.x();
        // proceeds with next point.
        index_it++;
        N++;
        // record segment endpoint    
        if (xmin > (dir.projection(p2)).x() || xmin == (dir.projection(p2)).x() && p2.y() < y1) { xmin = dir.projection(p2).x(); x1 = p2.x(); y1 = p2.y(); }
        if (xmax < (dir.projection(p2)).x() || xmax == (dir.projection(p2)).x() && p2.y() > y2) { xmax = dir.projection(p2).x(); x2 = p2.x(); y2 = p2.y(); }
    }
      if(plane->plane_normal().y() == 0) {
          disx /= N;
          line.line2 = Line_2(Point_2(disx, 1), Point_2(disx, -1));
      }
      else
          line.line2 = Line_2(k, -1, b/N);
      line.distance = sum_distances/N;
      line.s = line.line2.projection(Point_2(x1, y1));
      line.t = line.line2.projection(Point_2(x2, y2));
      line.normal = Vector_2(vx/N, vy/N);
      if(!point_2_range(line.s) || !point_2_range(line.t)){
          std::cout << "invalid line: "<< line.s << " " << line.t<<std::endl;
          return;
      }
      lines.push_back(line);
}

/*!
 * \description: Clean circle intersection.
 * \param circles detected cylinder projection
 * \param r radius threshold
 * \return
 */
void Clean(vector<Circle>& circles, float r)
{
     sort(circles.begin(), circles.end());
     for(int i = 0; i < circles.size(); i++){
          if(circles[i].radius > r){
            circles[i].flag = 0;
            continue;
       }
       if(circles[i].flag == 0) continue;
       for(int j = i+1; j < circles.size(); j++){
            if(circles[j].radius > r){
                circles[j].flag = 0;
                continue;
          }
          if(circles[j].flag == 0) continue;        
          if (sqrt((circles[i].center-circles[j].center).squared_length()) < circles[i].radius + circles[j].radius)
             circles[j].flag = 0;
       }
     }
}

/*!
 * \description: whether a point is in a polygon or not (for .ply file)
 * \param points polygon endpoints
 * \param poly polygon
 * \param point query point
 * \param h point height
 * \return 
 */
bool isin(vector<Point_3> &points, Point_3& poly, Point_2 point, double& h)
{
    int tag = 0;
    for(int i = 0; i < 3; i++){
        Segment_2 seg(Point_2(points[poly[i]].x(), points[poly[i]].y()), Point_2(points[poly[(i+1)%3]].x(), points[poly[(i+1)%3]].y()));
        if(seg.has_on(point)){
          tag = 1;
          break;
        }
        auto result = CGAL::intersection(seg, K::Ray_2(point,Vector_2(0,1)));
        if(result)
        if(auto p = boost::get<Point_2>(&*result))
          tag++;
    }
    if(tag%2 == 1){
      Point_3 p1 = points[poly[0]], p2 = points[poly[0]], p3 = points[poly[0]];
      for(int i = 0; i < 3; i++)
       if((points[poly[i]]-p1).squared_length() > 1e-6){
         p2 = points[poly[i]];
         break;
       }
      for(int i = 0; i < 3; i++)
       if((points[poly[i]]-p1).squared_length() > 1e-6 && (points[poly[i]]-p2).squared_length() > 1e-6){
         if((Line_3(p1, p2).projection(points[poly[i]]) - points[poly[i]]).squared_length() < 1e-6) continue;
         p3 = points[poly[i]];
         break;
       }
       if(p1 == p2 && p1 == p3)
          return false;
        Plane_3 pl(p1, p2, p3);
        auto result = CGAL::intersection(Line_3(Point_3(point.x(), point.y(), 0), Vector_3(0, 0, 1)), pl);
        if(result)
          if(auto *p = boost::get<Point_3>(&*result)){
              h = p->z();
              return true;
          }
        return false;
    }
    else
        return false;
}

/*!
 * \description: whether a point is in a polygon or not (for .off file)
 * \param points polygon endpoints
 * \param poly polygon
 * \param point query point
 * \param h point height
 * \return 
 */
bool isin(vector<Point_3> &points, vector<int>& poly, Point_2 point, double& h){
    int tag = 0;
    if(poly.size() < 3)
        return false;
    for(int i = 0; i < poly.size(); i++){
        Segment_2 seg(Point_2(points[poly[i]].x(), points[poly[i]].y()), Point_2(points[poly[(i+1)%poly.size()]].x(), points[poly[(i+1)%poly.size()]].y()));
        if(seg.has_on(point)){
          tag = 1;
          break;
        }
        auto result = CGAL::intersection(seg, K::Ray_2(point,Vector_2(0,1)));
        if(result)
        if(auto p = boost::get<Point_2>(&*result))
          tag++;
    }
    if(tag%2 == 1){
      Point_3 p1 = points[poly[0]], p2 = points[poly[0]], p3 = points[poly[0]];
      for(auto p:poly)
        if((points[p]-p1).squared_length() > 1e-6){
            p2 = points[p];
            break;
       }
      for(auto p:poly)
        if((points[p]-p1).squared_length() > 1e-6 && (points[p]-p2).squared_length() > 1e-6){
            if((Line_3(p1, p2).projection(points[p]) - points[p]).squared_length() < 1e-6) continue;
            p3 = points[p];
            break;
       }
       if(p1 == p2 && p1 == p3)
          return false;
        Plane_3 pl(p1, p2, p3);
        auto result = CGAL::intersection(Line_3(Point_3(point.x(), point.y(), 0), Vector_3(0, 0, 1)), pl);
        if(result)
          if(auto *p = boost::get<Point_3>(&*result)){
              h = p->z();
              return true;
          }
        return false;
    }
    else
        return false;
}


/*!
 * \description: Octagonal fitting cylinder.
 * \param circles detected cylinder projection
 * \param wdir file save/load directory
 * \return
 */
bool Toplane(vector<Circle>& circles, string wdir)
{
    int num = 0;
    vector<Point_2> vertexes;
    // generate cylinder file
    ofstream ofss(wdir + "cylinder_final");
    for(int i = 0; i < circles.size(); i ++)
    {
        if(circles[i].flag == 0) continue;
        num++;
        // 8 points
        Vector_2 add[8] = {Vector_2(0, circles[i].radius), Vector_2(circles[i].radius/1.414,circles[i].radius/1.414), 
                          Vector_2(circles[i].radius, 0), Vector_2(circles[i].radius/1.414, -circles[i].radius/1.414), 
                          Vector_2(0, -circles[i].radius), Vector_2(-circles[i].radius/1.414, -circles[i].radius/1.414), 
                          Vector_2(-circles[i].radius, 0), Vector_2(-circles[i].radius/1.414, circles[i].radius/1.414)};
        for(int j = 0; j < 8; j++)
            vertexes.push_back(circles[i].center + add[j]);
        for(int j = 8*i + 0; j < 8*i + 8 -1; j++)
            ofss << setiosflags(ios::fixed) << setprecision(8) << vertexes[j].x() << " " << vertexes[(j+1)%(8*(i+1))].x() << " " << vertexes[j].y() << " " << vertexes[(j+1)%(8*(i+1))].y() << "\n";
        ofss << setiosflags(ios::fixed) << setprecision(8) << vertexes[8*i + 7].x() << " " << vertexes[8*i + 0].x() << " " << vertexes[8*i + 7].y() << " " << vertexes[8*i + 0].y() << "\n";
    }
    std::cout << num << " cylinders are remained."<<std::endl; 
    ofss.close();

    // generate height according to floor.off ceiling.off
    vector<Point_2> height(vertexes.size()); // {max, min}
    ifstream floor(wdir + "floor.off");
    ifstream ceil(wdir + "ceiling.off");
    vector<Point_3> pointsf;
    vector<vector<int>> polysf;
    vector<Point_3> pointsc;
    vector<vector<int>> polysc;
    int vn, fn, o;
    string coff;
    double x,y,z;
    // floor.off
    floor >> coff;
    floor >> vn >> fn >> o;
    while(vn-->0){
      floor >> x >> y >> z;
      pointsf.push_back(Point_3(x, y, z));
    }
    while(fn-->0){
      floor >> o;
      vector<int> poly;
      while(o-->0){
        floor >> vn;
        poly.push_back(vn);
      }
      polysf.push_back(poly);
    }
    // ceiling.off
    ceil >> coff;
    ceil >> vn >> fn >> o;
    while(vn-->0){
      ceil >> x >> y >> z;
      pointsc.push_back(Point_3(x, y, z));
    }
    while(fn-->0){
      ceil >> o;
      vector<int> poly;
      while(o-->0){
        ceil >> vn;
        poly.push_back(vn);
      }
      polysc.push_back(poly);
    }
    for(int i = 0; i < vertexes.size(); i++){
       double hc = 1e9, hf = 1e9;
       for(int j = 0; j < polysf.size(); j++){
         if(isin(pointsf, polysf[j], vertexes[i], hf))
            break;
       }
       for(int j = 0; j < polysc.size(); j++){
         if(isin(pointsc, polysc[j], vertexes[i], hc)){
            break;
         }
       }
       if(hc == 1e9 || hf == 1e9){
            // LOG(INFO) << vertexes[i] << " Cylinder is failed.";
            return false;
       }
       height[i] = Point_2(hc, hf);
    }
/*
    // generate height according to floor_vec.ply ceiling_vec.ply
    vector<Point_2> height(vertexes.size()); // {max, min}
    string floor(wdir + "floor_vec.ply");
    string ceiling(wdir + "ceiling_vec.ply");
    vector<Point_3> pointsf;
    vector<vector<int>> polysf;
    vector<Point_3> pointsc;
    vector<vector<int>> polysc;
    Mesh<Point_3> meshf,meshc;
    PLYMeshLoad(floor, meshf);
    PLYMeshLoad(ceiling, meshc);

    for(int i = 0; i < vertexes.size(); i++){
        double hc = 1e9, hf = 1e9;
        for(int j = 0; j < meshf.faces.size(); j++){
          if(isin(meshf.vertices, meshf.faces[j], vertexes[i], hf))
              break;
        }
        for(int j = 0; j < meshc.faces.size(); j++){
          if(isin(meshc.vertices, meshc.faces[j], vertexes[i], hc)){
              break;
          }
        }
        if(hc==1e9 || hf==1e9){
          LOG(INFO) << vertexes[i] << " Cylinder failed.";
          return false;
        }
        height[i] = Point_2(hc, hf);
    }
*/
  // obtain height for each vertex, 8 vertexes -> 1 cylinder
  vector<Point_3> points;
  vector<vector<int>> polygons;
  for(int i = 0; i < vertexes.size(); i=i+8){
    for(int j = 0; j < 8; j++){
      Point_3 p1 = Point_3(vertexes[i+j].x(), vertexes[i+j].y(), height[i+j].x());
      Point_3 p2 = Point_3(vertexes[i+j].x(), vertexes[i+j].y(), height[i+j].y());
      Point_3 p3 = Point_3(vertexes[i+(j+1)%8].x(), vertexes[i+(j+1)%8].y(), height[i+(j+1)%8].y());
      Point_3 p4 = Point_3(vertexes[i+(j+1)%8].x(), vertexes[i+(j+1)%8].y(), height[i+(j+1)%8].x());
      int id1, id2, id3, id4;
      auto it = find(points.begin(), points.end(), p1);
      id1 = std::distance(points.begin(), it);
      if(it == points.end())
        points.push_back(p1);

      it = find(points.begin(), points.end(), p2);
      id2 = std::distance(points.begin(), it);
      if(it == points.end())
        points.push_back(p2);

      it = find(points.begin(), points.end(), p3);
      id3 = std::distance(points.begin(), it);
      if(it == points.end())
        points.push_back(p3);

      it = find(points.begin(), points.end(), p4);
      id4 = std::distance(points.begin(), it);
      if(it == points.end())
        points.push_back(p4);

      polygons.push_back({id1, id4, id3, id2});
    }
  }

  // generate cylinder.off
  ofstream ofs(wdir + "cylinder.off");
  ofs << "COFF\n" << points.size() << ' ' << polygons.size() << ' ' << "0\n";
  for (const auto &p : points)
    ofs << p.x() << ' ' << p.y() << ' ' << p.z() << '\n';
  for (const auto &plg : polygons) {
    ofs << plg.size();
    for (const auto &p : plg)
      ofs << ' ' << p;
    ofs << '\n';
  }
  ofs.close();
/*
  // generate cylinder_vec.ply
  string ofs = wdir + "cylinder_vec.ply";
  Mesh<Point_3> mesh;
  for(auto& p: points)
    mesh.vertices.push_back(p);
  for(auto& poly: polygons){
    for(int i = 2; i < poly.size(); i++){
        mesh.faces.push_back({poly[0], poly[i-1], poly[i]});
    }
  }
  PLYMeshSave(ofs, mesh, 1);
*/
  return true;
}

bool ransac_detection_p(Pwn_vector& points, float probability, int min_points, float epsilon, float cluster_epsilon, float normal_threshold, float cos_angle, vector<Line>& lines, string wdir, Pwn_vector& res_points, Pwn_vector& floor_points, Pwn_vector& ceiling_points, vector<Line>& floor_lines, vector<Line>& ceiling_lines, double min_z, double max_z, double whole_mean_z)
{
    // timer
    clock_t start, mid, end;
    start = clock(); 
    
    Efficient_ransac ransac;
    ransac.set_input(points);
    ransac.add_shape_factory<Plane>(); // rgister shapes for detection 
    Efficient_ransac::Parameters parameters; // sets parameters for shape detection.
    parameters.probability = probability; 
    parameters.min_points = min_points;
    parameters.epsilon = epsilon;  
    parameters.normal_threshold = normal_threshold;   
    if(cluster_epsilon)
      parameters.cluster_epsilon = cluster_epsilon; 

    // detects shapes
    for(int i = 0; i < 20; i++){
        ransac.detect(parameters);
        if(ransac.shapes().end() - ransac.shapes().begin() > 0) 
            continue;
    }

    if (ransac.shapes().end() - ransac.shapes().begin() == 0)
        return 0;
        
    std::cout<< ransac.shapes().end() - ransac.shapes().begin() << " detected planes, " << ransac.number_of_unassigned_points() << " unassigned points."<<std::endl;

    Efficient_ransac::Plane_range planes = ransac.planes();
    double re_angle = 15;
                   
    // regularize detected planes.
    //CGAL::regularize_planes(points,
    //                        Point_map(),
    //                        planes,
    //                        CGAL::Shape_detection::Plane_map<Traits>(),
    //                        CGAL::Shape_detection::Point_to_shape_index_map<Traits>(points, planes),
    //                        true,  // regularize parallelism
    //                        true,  // regularize orthogonality
    //                        true, //  regularize coplanarity
    //                        true,  // regularize Z-symmetry (default)
    //                        re_angle,
    //                        0.02);   // tolerance for parallelism / orthogonality
    int num = 0;
    Efficient_ransac::Plane_range::iterator it = planes.begin();
    Pwn_vector facade_points;
    vector<Line> cur_lines;
    while (it != planes.end()) {
        if (Plane* plane = dynamic_cast<Plane*>(it->get()))
        {
            Vector_3 normal = plane->plane_normal(); 
            // std::cout << abs(normal.z() / sqrt(normal.x() * normal.x() + normal.y() * normal.y() + normal.z() * normal.z())) << std::endl;
            // std::cout << normal.x() << " " << normal.y() << " " << normal.z() << std::endl;
            double angle_between_vectors =  abs(normal.z()/sqrt(normal.x()*normal.x()+normal.y()*normal.y()+normal.z()*normal.z()));
            if (angle_between_vectors <= cos_angle)
            {
              PtoS(plane, points, num, lines);
              num++;
            }
            else if (angle_between_vectors > 0.9 && angle_between_vectors <= 1.0) // floor or ceiling
            {
              PtoS(plane, points, num, floor_lines);
              num++;
            }
            else
            {
              it++; continue; // remove non-facade (default threshold = 80)
            }

            // if (abs(normal.z()/sqrt(normal.x()*normal.x()+normal.y()*normal.y()+normal.z()*normal.z())) > cos_angle) 
            // {
            //     it++; continue;
            // } // remove non-facade (default threshold = 80)
            // // store 2D projection lines and their supporting points
            // PtoS(plane, points, num, lines);
            // num++;
        }  
        // proceeds with next detected shape.
        it++;
    }

    for(int i = 0; i < lines.size(); i++)
    {
      if(lines[i].pointset.size()>1000)
      {
        float z_min = 1000;
        float z_max = -1000;
        for(auto& p: lines[i].pointset)
        {
          float cur_z = p[2];
          if(z_min>cur_z)
            z_min = cur_z;
          if(z_max<cur_z)
            z_max = cur_z;
        }
        if((z_max-z_min)>0.8)
          for(auto& p: lines[i].pointset)
          {
            // std::cout << p[0]<<" "<<p[1] <<" "<<p[2] << std::endl;
            res_points.push_back(Point_with_normal({p,Vector_3(0,1,0)}));
          }
            
      }

    
    }
    // std::cout << res_points.size() << std::endl;
    double range = (max_z - min_z) * 0.3;

    for(int i = 0; i < floor_lines.size(); i++)
    {
      if(floor_lines[i].pointset.size()>1000)
      {
        double mean_z = 0;

        for(auto& p: floor_lines[i].pointset)
        {
          float cur_z = p[2];
          mean_z += cur_z;
        }
        mean_z /= floor_lines[i].pointset.size();
        if(mean_z > whole_mean_z + range)
        {
          for(auto& p: floor_lines[i].pointset)
            ceiling_points.push_back(Point_with_normal({p,Vector_3(0,0,-1)}));
        }
        else if(mean_z < whole_mean_z - range)
        {
          for(auto& p: floor_lines[i].pointset)
            floor_points.push_back(Point_with_normal({p,Vector_3(0,0,1)}));
        }
      }
    }
    return 1;
}

bool ransac_detection_c(Pwn_vector& points, float radius, float probability, int min_points, float epsilon, float cluster_epsilon, float normal_threshold, float cos_angle, vector<Circle>& circles, string wdir)
{
    // timer
    clock_t start, end;
    start = clock(); 
    
    Efficient_ransac ransac;
    ransac.set_input(points);
    ransac.add_shape_factory<Cylinder>();
    Efficient_ransac::Parameters parameters;
    parameters.probability = probability; 
    parameters.min_points = min_points;
    parameters.epsilon = epsilon;  
    parameters.normal_threshold = normal_threshold;   
    if (cluster_epsilon)
        parameters.cluster_epsilon = cluster_epsilon; 

    ransac.detect(parameters);
    // LOG(INFO) << ransac.shapes().end() - ransac.shapes().begin() << " detected cylinders, "
    //     << ransac.number_of_unassigned_points()
    //     << " unassigned points." ;

    if (ransac.shapes().end() - ransac.shapes().begin() == 0)
    {
        // LOG(INFO) << "No cylinder is detected.";
        return 0;
    }

    Efficient_ransac::Shape_range cylinders = ransac.shapes();
    int num = 0;
    Efficient_ransac::Shape_range::iterator it = cylinders.begin();
    while (it != cylinders.end()) {
        if (Cylinder* cylinder = dynamic_cast<Cylinder*>(it->get()))
        {
            Line_3 axis = cylinder->axis();   
            if (abs(axis.to_vector().z()/sqrt(axis.to_vector().x()*axis.to_vector().x()+axis.to_vector().y()*axis.to_vector().y()+axis.to_vector().z()*axis.to_vector().z())) < cos_angle) {it++; continue;}
            double mid = 0;
            int num_c = 0;// find the middle height!
            for(auto p:cylinder->indices_of_assigned_points()){
                mid += points[p].first.z();
                num_c++;
            }    
            Plane_3 mid_p(Point_3(0,0,mid/num_c), Vector_3(0,0,1)); 
            auto result = CGAL::intersection(mid_p, axis);    
            if (result)
                if(const Point_3* pt = boost::get<Point_3>(&*result))	
                    circles.push_back(Circle(Point_2(pt->x(), pt->y()), cylinder->radius(),cylinder->indices_of_assigned_points().end()-cylinder->indices_of_assigned_points().begin(), 0, 0));
                else
                    std::cout<< "Cylinder detection is failed!";       
        }  
        it++;
        num++;
    }
    // LOG(INFO) << num << " cylinders are generated. ";
    
    // exclude intersection circles
    Clean(circles, radius);
    // valid circles produce eight planes
    if(!Toplane(circles, wdir))
        return 0;
    // save cylinder file
    string fname = wdir + "cylinder";
    ofstream ofs(fname);
    for(int i = 0; i < circles.size(); i++)
    {
        if (circles[i].flag == 0) continue;
        ofs << circles[i].center.x() << " " << circles[i].center.y() <<" "<< circles[i].radius << "\n";
    }
    ofs.close();

    end = clock();
    // LOG(INFO) << "Cylinder reconstruction time: " << (float)(end-start)/CLOCKS_PER_SEC << "s";
    return 1;
}
