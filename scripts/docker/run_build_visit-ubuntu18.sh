echo "yes" | ./build_visit3_4_2 --required --optional --mesagl --vtk9 --mpich --uintah --no-moab --no-pyside --no-visit --thirdparty-path /home/visit/third-party --makeflags -j4; python3 build_visit_docker_cleanup.py
