/**
GPL (v3+) License

This file is part of the code accompanying the paper
PrimiTect: Fast Continuous Hough Voting for Primitive Detection
by C. Sommer, Y. Sun, E. Bylow and D. Cremers,
accepted for publication in the IEEE International Conference on Robotics and Automation (ICRA) 2020.

Copyright (c) 2019, Christiane Sommer.
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <bitset>
#include <Eigen/Dense>
#include <CLI/CLI.hpp>
#include "Timer.h"
#include "pcd/PointCloudData.h"
#include "shapefit/SphereOptimizer.h"

int main(int argc, char** argv) {
    
    Timer T;
    
    // parse setttings from command line
    std::string folder, dfolder, stamp_file = "stamps.dat", stamp;

    CLI::App app{"Find pseudo-GT sphere parameters from point cloud data"};
    app.add_option("--folder", folder, "folder of input point clouds")->required();
    app.add_option("--stamps", stamp_file, "file containing stamps of all point clouds");
    app.add_option("--dfolder", dfolder, "folder containing detection results")->required();

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }
        
    std::ifstream infile(folder + stamp_file);
    while (!infile.eof()) {    
        infile >> stamp;
        std::cout << stamp << std::endl;
        std::ifstream infile_det(dfolder + stamp + "/" + stamp + "_o100111.dat");
        PointCloudData pc(folder + stamp + ".ply");
        std::vector<Sphere<double>*> spheres_vec;
        float votes;
        Eigen::Vector3d c;
        double r;
        size_t counter = 0;
        infile_det >> votes;
        bool conv;
        while (!infile_det.eof()) {
            infile_det >> c[0] >> c[1] >> c[2] >> r >> votes;
            Sphere<double>* S = new Sphere<double>(c, r);
            SphereOptimizer<PointCloudData> so(S);
            T.tic();
            conv = so.optimize(pc);
            T.toc();
            if (conv)
                spheres_vec.push_back(S);
            else
                delete S;
            if (votes < 500)
                break;
        }
        std::ofstream outfile(folder + stamp + ".info");
        outfile << "# stamp\n";
        outfile << "# num_spheres\n";
        outfile << "# cx cy cz r\n";
        outfile << stamp << "\n";
        outfile << spheres_vec.size() << "\n";
        std::cout << spheres_vec.size() << std::endl;
        for (auto S : spheres_vec) {
            outfile << (*S) << "\n";
            delete S;
        }
        outfile.close();
        
    }
    infile.close();
    
    return 0;
}
