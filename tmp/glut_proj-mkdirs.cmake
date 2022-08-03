# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/camport3/sample/common/cloud_viewer/freeglut"
  "E:/camport3/sample/build/freeglut_build"
  "E:/camport3/sample"
  "E:/camport3/sample/tmp"
  "E:/camport3/sample/src/glut_proj-stamp"
  "E:/camport3/sample/src"
  "E:/camport3/sample/src/glut_proj-stamp"
)

set(configSubDirs Debug;Release;MinSizeRel;RelWithDebInfo)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/camport3/sample/src/glut_proj-stamp/${subDir}")
endforeach()
