rm -rf _build
mkdir _build && cd _build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j4
