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

#include "WorkflowRecord.hpp"
#include "WorkflowRecord_Impl.hpp"

#include "JoinRecord.hpp"
#include "ProblemRecord.hpp"
#include "ProjectDatabase.hpp"

#include "../runmanager/lib/RunManager.hpp"
#include "../runmanager/lib/Workflow.hpp"

#include "../utilities/core/Assert.hpp"

#include <boost/optional/optional.hpp>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

namespace openstudio {
namespace project {

namespace detail {

  WorkflowRecord_Impl::WorkflowRecord_Impl(const runmanager::Workflow& workflow,
                                           ProblemRecord& problemRecord,
                                           int workflowIndex)
    : ObjectRecord_Impl(problemRecord.projectDatabase(),
                        workflow.uuid(),
                        "",
                        "",
                        "",
                        createUUID()),
      m_problemRecordId(problemRecord.id()),
      m_workflowIndex(workflowIndex)
  {
    openstudio::runmanager::RunManager runManager = problemRecord.projectDatabase().runManager();
    try {
      m_runManagerWorkflowKey = runManager.persistWorkflow(workflow);
    }
    catch (const std::exception& e) {
      LOG(Error,"Could not persist workflow, because '" << e.what() << "'.");
    }
  }

  WorkflowRecord_Impl::WorkflowRecord_Impl(const QSqlQuery& query, const ProjectDatabase& projectDatabase)
    : ObjectRecord_Impl(projectDatabase, query)
  {
    OS_ASSERT(query.isValid());
    OS_ASSERT(query.isActive());
    OS_ASSERT(query.isSelect());

    QVariant value;

    value = query.value(WorkflowRecordColumns::runManagerWorkflowKey);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_runManagerWorkflowKey = value.toString().toStdString();

    value = query.value(WorkflowRecordColumns::problemRecordId);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_problemRecordId = value.toInt();

    value = query.value(WorkflowRecordColumns::workflowIndex);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_workflowIndex = value.toInt();

  }

  std::string WorkflowRecord_Impl::databaseTableName() const {
    return WorkflowRecord::databaseTableName();
  }

  boost::optional<ObjectRecord> WorkflowRecord_Impl::parent() const {
    return problemRecord();
  }

  std::vector<ObjectRecord> WorkflowRecord_Impl::children() const
  {
    return std::vector<ObjectRecord>();
  }

  std::vector<ObjectRecord> WorkflowRecord_Impl::resources() const
  {
    return std::vector<ObjectRecord>();
  }

  std::vector<JoinRecord> WorkflowRecord_Impl::joinRecords() const
  {
    return std::vector<JoinRecord>();
  }

  void WorkflowRecord_Impl::saveRow(const std::shared_ptr<QSqlDatabase> &database)
  {
    QSqlQuery query(*database);
    this->makeUpdateByIdQuery<WorkflowRecord>(query);
    this->bindValues(query);
    assertExec(query);
  }

  ProblemRecord WorkflowRecord_Impl::problemRecord() const {
    ProjectDatabase database = projectDatabase();
    return ProblemRecord::getProblemRecord(m_problemRecordId,database).get();
  }

  std::string WorkflowRecord_Impl::runManagerWorkflowKey() const {
    return m_runManagerWorkflowKey;
  }

  int WorkflowRecord_Impl::workflowIndex() const {
    return m_workflowIndex;
  }

  openstudio::runmanager::Workflow WorkflowRecord_Impl::workflow() const {
    ProjectDatabase projectDatabase = this->projectDatabase();

    openstudio::runmanager::Workflow result;
    openstudio::runmanager::RunManager runManager = projectDatabase.runManager();
    try {
      result = runManager.loadWorkflow(m_runManagerWorkflowKey);
      result.setUUID(handle());
    }
    catch (...) {
      LOG(Error,"Unable to load workflow from RunManager using key "
          << m_runManagerWorkflowKey << ". Returning empty Workflow.");
    }

    return result;
  }

  void WorkflowRecord_Impl::bindValues(QSqlQuery& query) const
  {
    ObjectRecord_Impl::bindValues(query);

    query.bindValue(WorkflowRecordColumns::runManagerWorkflowKey, toQString(m_runManagerWorkflowKey));
    query.bindValue(WorkflowRecordColumns::problemRecordId, m_problemRecordId);
    query.bindValue(WorkflowRecordColumns::workflowIndex, m_workflowIndex);
  }

  void WorkflowRecord_Impl::setLastValues(const QSqlQuery& query, ProjectDatabase& projectDatabase)
  {
    ObjectRecord_Impl::setLastValues(query, projectDatabase);

    QVariant value;

    value = query.value(WorkflowRecordColumns::runManagerWorkflowKey);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_lastRunManagerWorkflowKey = value.toString().toStdString();

    value = query.value(WorkflowRecordColumns::problemRecordId);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_lastProblemRecordId = value.toInt();

    value = query.value(WorkflowRecordColumns::workflowIndex);
    OS_ASSERT(value.isValid() && !value.isNull());
    m_lastWorkflowIndex = value.toInt();
  }

  bool WorkflowRecord_Impl::compareValues(const QSqlQuery& query) const
  {
    bool result = true;

    result = result && ObjectRecord_Impl::compareValues(query);

    QVariant value;

    value = query.value(WorkflowRecordColumns::runManagerWorkflowKey);
    OS_ASSERT(value.isValid() && !value.isNull());
    result = result && (m_runManagerWorkflowKey == value.toString().toStdString());

    value = query.value(WorkflowRecordColumns::problemRecordId);
    OS_ASSERT(value.isValid() && !value.isNull());
    result = result && (m_problemRecordId == value.toInt());

    value = query.value(WorkflowRecordColumns::workflowIndex);
    OS_ASSERT(value.isValid() && !value.isNull());
    result = result && (m_workflowIndex == value.toInt());

    return result;
  }

  void WorkflowRecord_Impl::saveLastValues()
  {
    ObjectRecord_Impl::saveLastValues();

    m_lastRunManagerWorkflowKey = m_runManagerWorkflowKey;
    m_lastProblemRecordId = m_problemRecordId;
    m_lastWorkflowIndex = m_workflowIndex;
  }

  void WorkflowRecord_Impl::revertToLastValues()
  {
    ObjectRecord_Impl::revertToLastValues();

    m_runManagerWorkflowKey = m_lastRunManagerWorkflowKey;
    m_problemRecordId = m_lastProblemRecordId;
    m_workflowIndex = m_lastWorkflowIndex;
  }

} // detail

WorkflowRecord::WorkflowRecord(const runmanager::Workflow& workflow,
                               ProblemRecord& problemRecord,
                               int workflowIndex)
  : ObjectRecord(std::shared_ptr<detail::WorkflowRecord_Impl>(
                   new detail::WorkflowRecord_Impl(workflow,
                                                   problemRecord,
                                                   workflowIndex)),
                 problemRecord.projectDatabase())
{
  OS_ASSERT(getImpl<detail::WorkflowRecord_Impl>());
}

WorkflowRecord::WorkflowRecord(const QSqlQuery& query,ProjectDatabase& database)
  : ObjectRecord(std::shared_ptr<detail::WorkflowRecord_Impl>(
                   new detail::WorkflowRecord_Impl(query,database)),
                   database)
{
  OS_ASSERT(getImpl<detail::WorkflowRecord_Impl>());
}

std::string WorkflowRecord::databaseTableName()
{
  return "WorkflowRecords";
}

UpdateByIdQueryData WorkflowRecord::updateByIdQueryData() {
  static UpdateByIdQueryData result;
  if (result.queryString.empty()) {
    // numeric column identifiers
    result.columnValues = ColumnsType::getValues();

    // query string
    std::stringstream ss;
    ss << "UPDATE " << databaseTableName() << " SET ";
    int expectedValue = 0;
    for (auto it = result.columnValues.begin(),
         itend = result.columnValues.end(); it != itend; ++it)
    {
      // require 0 based columns, don't skip any
      OS_ASSERT(*it == expectedValue);
      // column name is name, type is description
      ss << ColumnsType::valueName(*it) << "=:" << ColumnsType::valueName(*it);
      // is this the last column?
      auto nextIt = it;
      ++nextIt;
      if (nextIt == itend) {
        ss << " ";
      }
      else {
        ss << ", ";
      }
      ++expectedValue;
    }
    ss << "WHERE id=:id";
    result.queryString = ss.str();

    // null values
    for (const auto & columnValue : result.columnValues)
    {
      // bind all values to avoid parameter mismatch error
      if (istringEqual(ColumnsType::valueDescription(columnValue), "INTEGER")) {
        result.nulls.push_back(QVariant(QVariant::Int));
      }
      else {
        result.nulls.push_back(QVariant(QVariant::String));
      }
    }
  }
  return result;
}

void WorkflowRecord::updatePathData(ProjectDatabase database,
                                    const openstudio::path& originalBase,
                                    const openstudio::path& newBase)
{}

boost::optional<WorkflowRecord> WorkflowRecord::factoryFromQuery(const QSqlQuery& query,
                                                                 ProjectDatabase& database)
{
  OptionalWorkflowRecord result;
  try {
    result = WorkflowRecord(query,database);
  }
  catch (const std::exception& e) {
    LOG(Error,"Unable to construct WorkflowRecord from query, because '"
        << e.what() << "'.");
  }
  return result;
}

std::vector<WorkflowRecord> WorkflowRecord::getWorkflowRecords(ProjectDatabase& projectDatabase)
{
  std::vector<WorkflowRecord> result;

  QSqlQuery query(*(projectDatabase.qSqlDatabase()));
  query.prepare(toQString("SELECT * FROM " + WorkflowRecord::databaseTableName()));
  assertExec(query);
  while (query.next()) {
    result.push_back(projectDatabase.getFromQuery<WorkflowRecord>(query));
  }

  return result;
}

boost::optional<WorkflowRecord> WorkflowRecord::getWorkflowRecord(int id, ProjectDatabase& projectDatabase)
{
  boost::optional<WorkflowRecord> result;

  QSqlQuery query(*(projectDatabase.qSqlDatabase()));
  query.prepare(toQString("SELECT * FROM " + WorkflowRecord::databaseTableName() + " WHERE id=:id"));
  query.bindValue(":id", id);
  assertExec(query);
  if (query.first()) {
    result = projectDatabase.getFromQuery<WorkflowRecord>(query);
  }

  return result;
}

ProblemRecord WorkflowRecord::problemRecord() const {
  return getImpl<detail::WorkflowRecord_Impl>()->problemRecord();
}

std::string WorkflowRecord::runManagerWorkflowKey() const {
  return getImpl<detail::WorkflowRecord_Impl>()->runManagerWorkflowKey();
}

int WorkflowRecord::workflowIndex() const {
  return getImpl<detail::WorkflowRecord_Impl>()->workflowIndex();
}

runmanager::Workflow WorkflowRecord::workflow() const {
  return getImpl<detail::WorkflowRecord_Impl>()->workflow();
}

/// @cond
WorkflowRecord::WorkflowRecord(std::shared_ptr<detail::WorkflowRecord_Impl> impl)
  : ObjectRecord(impl)
{
  OS_ASSERT(getImpl<detail::WorkflowRecord_Impl>());
}
/// @endcond

bool WorkflowRecordWorkflowIndexLess::operator()(const WorkflowRecord& left, 
                                                 const WorkflowRecord& right) const
{
  return (left.workflowIndex() < right.workflowIndex());
}

} // project
} // openstudio
