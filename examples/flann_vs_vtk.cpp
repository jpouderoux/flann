#include <flann/flann.h>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#include <vtkIdList.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkStaticPointLocator.h>
#include <vtkTimerLog.h>

int main()
{
  int nbPoints = 1000000; // # of initial points
  int testCount = 1000;   // # of search tests
  int nn = 1;             // number of nearest neighbors to search for
  std::vector<int> result(testCount * nn);
  std::vector<double> dists(testCount * nn);
  std::vector<double> dataset(nbPoints * 3);

  //----------------------------------------------------------------------------
  // Generate input points distribution
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  points->SetNumberOfPoints(nbPoints);
  vtkNew<vtkPolyData> pd;
  pd->SetPoints(points.Get());
  for (int i = 0; i < nbPoints; i++)
  {
    const double point[3] = { vtkMath::Random(), vtkMath::Random(), vtkMath::Random() };
    for (int j = 0; j < 3; j++) dataset[i * 3 + j] = point[j];
    points->SetPoint(i, point);
  }

  vtkNew<vtkTimerLog> log;

  //----------------------------------------------------------------------------
  // Test FLANN
  {
  cout << "FLANN" << endl;
  float speedup;
  struct FLANNParameters p;
  p = DEFAULT_FLANN_PARAMETERS;
  p.cores = 4;
 /* p.algorithm = FLANN_INDEX_KDTREE;
  p.trees = 8;
  p.log_level = FLANN_LOG_INFO;
  p.checks = 64;*/
  log->StartTimer();
  cout << "Computing index..." << endl;
  flann_index_t index_id = flann_build_index_double(dataset.data(), nbPoints, 3, &speedup, &p);
  log->StopTimer();
  cout << "Creating time of " << nbPoints << " points in " << log->GetElapsedTime() << "s" << endl;

  log->StartTimer();
  vtkMath::RandomSeed(4012);
  for (int i = 0; i < testCount; i++)
  {
    double point[3] = { vtkMath::Random(), vtkMath::Random(), vtkMath::Random() };
    flann_find_nearest_neighbors_index_double(index_id, point, 1, result.data(), dists.data(), nn, &p);
    if (i < 10) cout << result[0] << endl;
  }

  log->StopTimer();
  cout << "Search for " << testCount << " points in " << log->GetElapsedTime() << "s" << endl;
  flann_free_index(index_id, &p);
  }

  //----------------------------------------------------------------------------
  // Test vtkPointLocator
  {
  cout << "vtkPointLocator" << endl;
  vtkNew<vtkPointLocator> locator;
  locator->SetDataSet(pd.Get());

  log->StartTimer();
  cout << "Building locator." << endl;
  locator->BuildLocator();
  log->StopTimer();
  cout << "Creating time of " << nbPoints << " points in " << log->GetElapsedTime() << "s" << endl;

  log->StartTimer();
  vtkMath::RandomSeed(4012);
  for (int i = 0; i < testCount; i++)
  {
    double point[3] = { vtkMath::Random(), vtkMath::Random(), vtkMath::Random() };
    vtkIdType index_id = locator->FindClosestPoint(point);
    if (i < 10) cout << index_id << endl;
  }

  log->StopTimer();
  cout << "Searched for " << testCount << " points in " << log->GetElapsedTime() << "s" << endl;
  }

  //----------------------------------------------------------------------------
  // Test vtkStaticPointLocator
  {
  cout << "vtkStaticPointLocator" << endl;
  vtkNew<vtkStaticPointLocator> locator;
  locator->SetDataSet(pd.Get());

  log->StartTimer();
  cout << "Building locator." << endl;
  locator->BuildLocator();
  log->StopTimer();
  cout << "Creating time of " << nbPoints << " points in " << log->GetElapsedTime() << "s" << endl;

  log->StartTimer();
  vtkMath::RandomSeed(4012);
  for (int i = 0; i < testCount; i++)
  {
    double point[3] = { vtkMath::Random(), vtkMath::Random(), vtkMath::Random() };
    vtkIdType index_id = locator->FindClosestPoint(point);
    if (i < 10) cout << index_id << endl;
  }

  log->StopTimer();
  cout << "Searched for " << testCount << " points in " << log->GetElapsedTime() << "s" << endl;
  }

  return EXIT_SUCCESS;
}
