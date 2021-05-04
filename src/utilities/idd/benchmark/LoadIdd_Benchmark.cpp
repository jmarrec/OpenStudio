#include <benchmark/benchmark.h>

#include "../IddFile_Impl.hpp"
#include "../IddFile.hpp"
#include "../../core/Filesystem.hpp"
#include "../../core/Assert.hpp"

#include <resources.hxx>

#include <OpenStudio.hxx>

using namespace openstudio;

static void BM_ParseEnergyPlusIdd(benchmark::State& state) {

  path iddPath = resourcesPath() / toPath("energyplus/ProposedEnergy+.idd");

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    openstudio::filesystem::ifstream inFile(iddPath);
    BOOST_ASSERT(inFile ? true : false);
    OptionalIddFile loadedIddFile = IddFile::load(inFile);
    BOOST_ASSERT(loadedIddFile);
    inFile.close();
  }
}

static void BM_ParseOpenStudioIdd(benchmark::State& state) {

  path iddPath = resourcesPath() / toPath("model/OpenStudio.idd");

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    openstudio::filesystem::ifstream inFile(iddPath);
    BOOST_ASSERT(inFile ? true : false);
    OptionalIddFile loadedIddFile = IddFile::load(inFile);
    BOOST_ASSERT(loadedIddFile);
    inFile.close();
  }
}

BENCHMARK(BM_ParseEnergyPlusIdd);
BENCHMARK(BM_ParseOpenStudioIdd);
