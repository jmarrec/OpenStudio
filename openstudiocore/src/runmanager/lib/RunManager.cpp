/***********************************************************************************************************************
 *  OpenStudio(R), Copyright (c) 2008-2016, Alliance for Sustainable Energy, LLC. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *  disclaimer.
 *
 *  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *  following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
 *  products derived from this software without specific prior written permission from the respective party.
 *
 *  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative
 *  works may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without
 *  specific prior written permission from Alliance for Sustainable Energy, LLC.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************************************************/

#include "RunManager.hpp"
#include "RunManagerStatus.hpp"
#include "JSON.hpp"

#include "Workflow.hpp"
#include "RunManager_Impl.hpp"
#include <map>

#include "../../utilities/core/Path.hpp"
#include "../../utilities/core/PathHelpers.hpp"
#include "../../utilities/core/Application.hpp"

#include "../../model/Model.hpp"
#include "../../model/ShadowCalculation.hpp"
#include "../../model/ConvergenceLimits.hpp"
#include "../../model/Surface.hpp"
#include "../../model/SubSurface.hpp"
#include "../../model/Timestep.hpp"
#include "../../model/ShadowCalculation_Impl.hpp"
#include "../../model/ConvergenceLimits_Impl.hpp"
#include "../../model/Surface_Impl.hpp"
#include "../../model/SubSurface_Impl.hpp"
#include "../../model/Timestep_Impl.hpp"
#include "../../model/DesignDay.hpp"
#include "../../model/DesignDay_Impl.hpp"


#include <QThread>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

#include <boost/filesystem.hpp>

namespace openstudio {
namespace runmanager {

  /// Internal handler for looking up and keeping references to previously open DB files for RunManager
  /// \todo may need to make this thread save soon it was designed so only one mutex would be needed for that
  struct RunManager::DB_Handler
  {
    typedef std::map<openstudio::path, std::weak_ptr<detail::RunManager_Impl> > DB_Map;

    DB_Map m_dbs;
    QMutex m_mutex;

    void cull_dbs()
    {
      QMutexLocker l(&m_mutex);
      auto itr = m_dbs.begin();
      auto end = m_dbs.end();

      while (itr != end)
      {
        // If it's expired, we don't need to keep a reference to it
        if (itr->second.expired())
        {
          auto to_erase = itr;
          ++itr; // skip to the next before erasing the current
          m_dbs.erase(to_erase);
        } else {
          ++itr;
        }
      }
    }

    std::shared_ptr<detail::RunManager_Impl> get_impl(const openstudio::path &DB, bool t_new, bool t_paused, bool t_initui, bool t_tempdb, bool t_useStatusGUI)
    {
      // use complete path
      openstudio::path wDB = completeAndNormalize(DB);

      cull_dbs();
      QMutexLocker l(&m_mutex);
      // If this database file is already open just return the existing one
      // it may be premature to implement this, but it seems to make sense,
      // this way anyone in the same application with the same db file open
      // will be looking at the same list of queue items and such
      auto itr = m_dbs.find(wDB);

      if (itr != m_dbs.end())
      {
        try {
          std::shared_ptr<detail::RunManager_Impl> founddb = std::shared_ptr<detail::RunManager_Impl>(itr->second);
          if (t_new)
          {
            throw std::runtime_error("Unable to make new DB " + toString(wDB) + " file is already open");
          }

          return founddb;
        } catch ( const std::bad_weak_ptr & ) {
          m_dbs.erase(itr); // the current one is bad
          itr = m_dbs.end();
        }
      }

      if (t_new)
      {
        try {
          if (boost::filesystem::exists(wDB))
          {
            boost::filesystem::remove(wDB);
          }
        } catch (const boost::filesystem::filesystem_error &) {
          throw std::runtime_error("Unable to delete database file: " + toString(wDB));
        }
      }

      // We were unable to construct a valid shared_ptr from what we
      // did or did not have, so make a new one
      std::shared_ptr<detail::RunManager_Impl> impl(new detail::RunManager_Impl(wDB, t_paused, t_initui, t_tempdb, t_useStatusGUI));
      m_dbs.insert(std::make_pair(wDB, std::weak_ptr<detail::RunManager_Impl>(impl)));
      return impl;

    }
  };

  openstudio::path RunManager::generateTempPathName()
  {
#ifdef Q_OS_WIN
    const char *tmpdbname = tmpnam(0);
    return openstudio::tempDir()/toPath(std::string(tmpdbname));
#else
    char tmpdbname[1024];
    tmpdbname[1023] = 0;
    strncpy(tmpdbname, toString(openstudio::tempDir()/toPath(std::string("RMDBXXXXXX"))).c_str(), 1023 );
    int fd = mkstemp(tmpdbname);
    close(fd); // close the temp file created by mkstemp

    return toPath(std::string(tmpdbname));
#endif
  }

  RunManager::RunManager(const openstudio::path &DB, bool t_new, bool t_paused, bool t_initui, bool t_useStatusGUI)
    : m_impl(get_db_handler().get_impl(DB, t_new, t_paused, t_initui, false, t_useStatusGUI))
  {
  }

  RunManager::RunManager(bool t_paused, bool t_initui, bool t_useStatusGUI)
    : m_impl(get_db_handler().get_impl(generateTempPathName(), true, t_paused, t_initui, true, t_useStatusGUI))
  {
  }

  RunManager::~RunManager()
  {
    get_db_handler().cull_dbs();
  }

  void RunManager::waitForFinished(int t_msecs)
  {
    m_impl->waitForFinished(t_msecs);
  }

  openstudio::path RunManager::defaultDBLocation()
  {
    return toPath(QDir::homePath()) / toPath(".runmanager_default_db");
  }

  void RunManager::setOutOfDateRunnable()
  {
    return m_impl->setOutOfDateRunnable();
  }

  bool RunManager::outOfDate(const openstudio::runmanager::Job &job) const
  {
    return m_impl->outOfDate(job);
  }

  bool RunManager::enqueue(const Job &job, bool force, const openstudio::path &basePath)
  {
    return m_impl->enqueue(job, force, basePath);
  }

  boost::optional<Job> RunManager::enqueueOrReturnExisting(const Job &job,
                                                           bool force,
                                                           const openstudio::path &basePath)
  {
    return m_impl->enqueueOrReturnExisting(job,force,basePath);
  }

  bool RunManager::enqueue(const std::vector<openstudio::runmanager::Job> &jobs, bool force, const openstudio::path &t_basePath)
  {
    return m_impl->enqueue(jobs, force, t_basePath);
  }

  void RunManager::remove(const openstudio::runmanager::Job &job)
  {
    m_impl->remove(job);
  }

  openstudio::path RunManager::dbPath() const
  {
    return m_impl->dbPath();
  }

  bool RunManager::workPending() const
  {
    return m_impl->workPending();
  }

  RunManagerStatusHandle RunManager::showStatusDialog()
  {
    m_handle = RunManagerStatusHandle(m_impl->getStatusDialog(*this));
    return m_handle;
  }

  void RunManager::hideStatusDialog()
  {
    m_handle = RunManagerStatusHandle();
    m_impl->hideStatusDialog();
  }

  bool RunManager::paused() const 
  {
    return m_impl->paused();
  }

  void RunManager::setPaused(bool t_paused)
  {
    m_impl->setPaused(t_paused);
  }

  void RunManager::clearJobs()
  {
    m_impl->clearJobs();
  }

  /*
  void RunManager::waitForCompletion() const 
  {
    while (workPending())
    {
      openstudio::Application::instance().processEvents();
      QThread::yieldCurrentThread();
    }
  }
  */

  QAbstractItemModel *RunManager::getQItemModel()
  {
    return m_impl->getQItemModel();
  }

  bool RunManager::connect(const char *signal, 
      const QObject *receiver, const char *method, Qt::ConnectionType type) const
  {
    return QObject::connect(m_impl.get(), signal, receiver, method, type);
  }

  bool RunManager::connect(const QObject *sender, const char *signal, const char *method, Qt::ConnectionType type) const
  {
    return QObject::connect(sender, signal, m_impl.get(), method, type);
  }

  bool RunManager::disconnect( const char * signal, const QObject * receiver, const char * method)
  {
    return m_impl->disconnect(signal, receiver, method);
  }

  std::vector<openstudio::runmanager::Job> RunManager::getJobs() const
  {
    return m_impl->getJobs();
  }

  openstudio::runmanager::Job RunManager::getJob(const openstudio::UUID &t_uuid) const
  {
    return m_impl->getJob(t_uuid);
  }

  openstudio::runmanager::Job RunManager::getJob(const QModelIndex &t_index) const
  {
    return m_impl->getJob(t_index);
  }

  QModelIndex RunManager::getIndex(const Job &t_job) const
  {
    return m_impl->getIndex(t_job);
  }


  /// \returns a vector of all jobs contained in the QModelIndexList provided
  std::vector<openstudio::runmanager::Job> RunManager::getJobs(const QModelIndexList &t_indexes) const
  {
    return m_impl->getJobs(t_indexes);
  }


  void RunManager::updateJob(const openstudio::runmanager::Job &t_job, const openstudio::path &t_path)
  {
    LOG(Info, "We are re-creating the updating job as an externally managed job, we may want to reevaluate this");
    m_impl->updateJob(detail::JSON::toJob(detail::JSON::toJSON(t_job), true), t_path);
  }

  void RunManager::updateJob(const openstudio::UUID &t_uuid, const Job &t_job)
  {
    LOG(Info, "We are re-creating the updating job as an externally managed job, we may want to reevaluate this");
    m_impl->updateJob(t_uuid, detail::JSON::toJob(detail::JSON::toJSON(t_job), true));
  }

  void RunManager::updateJob(const openstudio::runmanager::Job &t_job)
  {
    LOG(Info, "We are re-creating the updating job as an externally managed job, we may want to reevaluate this");
    m_impl->updateJob(detail::JSON::toJob(detail::JSON::toJSON(t_job), true));
  }


  /// Load all of the jobs from the given JSON string, merging job trees
  void RunManager::updateJobs(const std::string &t_json, bool t_externallyManaged)
  {
    QVariant variant = loadJSON(t_json);
    VersionString version = extractOpenStudioVersion(variant);
    updateJobs(variant.toMap()["jobs"], version, t_externallyManaged);
  }

  /// Load all of the jobs from the given JSON structure represented by a QVariant,
  /// merging job trees
  void RunManager::updateJobs(const QVariant &t_variant, const VersionString &t_version, bool t_externallyManaged)
  {
    updateJobs(detail::JSON::toVectorOfJob(t_variant, t_version, t_externallyManaged));
  }

  /// merge job trees
  void RunManager::updateJobs(const std::vector<Job> &t_jobs)
  {
    m_impl->updateJobs(t_jobs);
  }

  void RunManager::loadJobs(const openstudio::path &t_path)
  {
    m_impl->loadJobs(t_path);
  }



  std::string RunManager::jobsToJson() const
  {
    return detail::JSON::toJSON(jobsForExport());
  }

  std::vector<Job> RunManager::jobsForExport() const
  {
    std::vector<Job> retval;

    std::vector<Job> currentJobs = getJobs();

    for (const auto & job : currentJobs)
    {
      // only parent jobs get saved
      if (!job.parent())
      {
        retval.push_back(job);
      }
    }

    LOG(Debug, "Returning jobs for export: " << retval.size());

    return retval;
  }


  openstudio::runmanager::ConfigOptions RunManager::getConfigOptions() const
  {
    return m_impl->getConfigOptions();
  }

  void RunManager::setConfigOptions(const openstudio::runmanager::ConfigOptions &co)
  {
    return m_impl->setConfigOptions(co);
  }

  void RunManager::raisePriority(const openstudio::runmanager::Job &t_job) 
  {
    m_impl->raisePriority(t_job);
  }

  void RunManager::lowerPriority(const openstudio::runmanager::Job &t_job) 
  {
    m_impl->lowerPriority(t_job);
  }

  void RunManager::raisePriority(const std::vector<Job> &t_jobs)
  {
    m_impl->raisePriority(t_jobs);
  }

  void RunManager::lowerPriority(const std::vector<Job> &t_jobs)
  {
    m_impl->lowerPriority(t_jobs);
  }


  void RunManager::showConfigGui(QWidget *parent)
  {
    m_impl->showConfigGui(parent);
  }

  void RunManager::showConfigGui()
  {
    m_impl->showConfigGui(nullptr);
  }

  RunManager::DB_Handler &RunManager::get_db_handler()
  {
    static DB_Handler m;
    return m;
  }
  
  std::map<std::string, double> RunManager::statistics() const
  {
    return m_impl->statistics();
  }

  std::string RunManager::persistWorkflow(const Workflow &t_wf)
  {
    return m_impl->persistWorkflow(t_wf);
  }

  std::vector<Workflow> RunManager::loadWorkflows() const
  {
    return m_impl->loadWorkflows();
  }

  Workflow RunManager::loadWorkflow(const std::string &key) const
  {
    return m_impl->loadWorkflow(key);
  }

  void RunManager::deleteWorkflows()
  {
    m_impl->deleteWorkflows();
  }

  Workflow RunManager::loadWorkflowByName(const std::string &t_name) const
  {
    return m_impl->loadWorkflowByName(t_name);
  }

  void RunManager::deleteWorkflowByName(const std::string &t_name)
  {
    m_impl->deleteWorkflowByName(t_name);
  }

  runmanager::Job RunManager::runWorkflow(const std::string &t_json, const openstudio::path &t_basePath, const openstudio::path &t_runPath, 
      const openstudio::runmanager::Tools &t_tools, const openstudio::runmanager::JSONWorkflowOptions &t_options)
  {
    return m_impl->runWorkflow(t_json, t_basePath, t_runPath, t_tools, t_options);
  }

  runmanager::Job RunManager::runWorkflow(const openstudio::path &t_jsonPath, const openstudio::path &t_basePath, const openstudio::path &t_runPath,
      const openstudio::runmanager::Tools &t_tools, const openstudio::runmanager::JSONWorkflowOptions &t_options)
  {
    return m_impl->runWorkflow(t_jsonPath, t_basePath, t_runPath, t_tools, t_options);
  }


  void RunManager::simplifyModelForPerformance(openstudio::model::Model &t_model)
  {

    // reset windows with wwr bands to simplify geometry
    std::vector<openstudio::model::Surface> surfaces = t_model.getConcreteModelObjects<openstudio::model::Surface>();

    for (auto & surface : surfaces)
    {
      if (surface.outsideBoundaryCondition() == "Outdoors"
          && surface.surfaceType() == "Wall")
      {
        double wwr = surface.windowToWallRatio();
        LOG(Debug, "Existing WWR: " << wwr);

        if (wwr > 0)
        {
          if (!surface.setWindowToWallRatio(wwr))
          {
            LOG(Warn, "Error setting WWR");
          }
        }
      }
    }

    /*
       Timestep,1; // timesteps per hour
       */

    openstudio::model::Timestep ts = t_model.getUniqueModelObject<openstudio::model::Timestep>();
    ts.setNumberOfTimestepsPerHour(1);

    /*
       ShadowCalculation,
       30,                                      !- Calculation Frequency
       15000;                                  !- Maximum Figures in Shadow Overlap Calculations
       */

    openstudio::model::ShadowCalculation sc =  t_model.getUniqueModelObject<openstudio::model::ShadowCalculation>();
    sc.setCalculationFrequency(30);


    /*
       ConvergenceLimits,
       10,                                      !- Minimum System Timestep {minutes}
       10;   // hvac iterations
       */

    openstudio::model::ConvergenceLimits cl =  t_model.getUniqueModelObject<openstudio::model::ConvergenceLimits>();
    cl.setMinimumSystemTimestep(10);
    cl.setMaximumHVACIterations(10);


    // clean up design days based on logic from openstudiolib
    std::vector<model::DesignDay> days99;
    std::vector<model::DesignDay> days99_6;
    std::vector<model::DesignDay> days2;
    std::vector<model::DesignDay> days1;
    std::vector<model::DesignDay> days0_4;

    bool unknownDay = false;

    for (model::DesignDay designDay : t_model.getConcreteModelObjects<model::DesignDay>()) {
      boost::optional<std::string> name;
      name = designDay.name();

      if( name ) 
      {
        LOG(Debug, "analyzing design day: " << *name);
        QString qname = QString::fromStdString(name.get()); 

        if( qname.contains("99%") ) 
        {
          days99.push_back(designDay);
        } 
        else if( qname.contains("99.6%") )
        {
          days99_6.push_back(designDay);
        }
        else if( qname.contains("2%") )
        {
          days2.push_back(designDay);
        }
        else if( qname.contains("1%") )
        {
          days1.push_back(designDay);
        }
        else if( qname.contains(".4%") )
        {
          days0_4.push_back(designDay);
        }
        else
        {
          LOG(Info, "Unknown day found: " << *name);
          unknownDay = true;
        }
      }

    }

    // Pick only the most stringent design points
    if( ! unknownDay )
    {
      if( days99_6.size() > 0 )
      {
        LOG(Debug, "removing 99% days: " << days99.size());
        for (model::DesignDay designDay : days99) {
          designDay.remove();
        }
      }

      if( days0_4.size() > 0 )
      {
        LOG(Debug, "removing 1% days: " << days1.size());
        for (model::DesignDay designDay : days1) {
          designDay.remove();
        }
        LOG(Debug, "removing 2% days: " << days2.size());
        for (model::DesignDay designDay : days2) {
          designDay.remove();
        }
      }
      else if( days1.size() > 0 )
      {
        LOG(Debug, "removing 2% days: " << days2.size());
        for (model::DesignDay designDay : days2) {
          designDay.remove();
        }
      }
    }
  }

}
}
