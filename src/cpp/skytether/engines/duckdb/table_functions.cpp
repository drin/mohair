  // ------------------------------
  // Bind functions

  static unique_ptr<FunctionData>
  BindingFnTranslateMohair( ClientContext&          context
                           ,TableFunctionBindInput& input
                           ,vector<LogicalType>&    return_types
                           ,vector<string>&         names) {
    if (input.inputs[0].IsNull()) {
      throw BinderException("from_substrait cannot be called with a NULL parameter");
    }
    string plan_msg { input.inputs[0].GetValueUnsafe<string>() };

    // Prepare a FunctionData instance to return
    auto fn_data = make_uniq<FnDataSubstraitTranslation>();
    fn_data->translator       = make_uniq<DuckDBTranslator>(context);
    fn_data->sys_plan         = fn_data->translator->TranslatePlanMessage(plan_msg);
    fn_data->enable_optimizer = GetOptimizationOption(context.config, input.named_parameters);

    // Set result schema (binding)
    return_types.emplace_back(LogicalType::VARCHAR);
    names.emplace_back("Physical Plan");

    return std::move(fn_data);
  }

  static unique_ptr<FunctionData>
  BindingFnExecuteMohair( ClientContext&          context
                         ,TableFunctionBindInput& input
                         ,vector<LogicalType>&    return_types
                         ,vector<string>&         names) {
    if (input.inputs[0].IsNull()) {
      throw BinderException("from_substrait cannot be called with a NULL parameter");
    }

    string plan_msg { input.inputs[0].GetValueUnsafe<string>() };

    auto fn_data = make_uniq<FnDataSubstraitExecution>();
    fn_data->translator = make_uniq<DuckDBTranslator>(context);
    fn_data->sys_plan   = fn_data->translator->TranslatePlanMessage(plan_msg);
    fn_data->plan_data  = make_shared_ptr<PreparedStatementData>(StatementType::SELECT_STATEMENT);

    Relation& duck_plan = *(fn_data->sys_plan->duck_plan);
    for (auto &column : duck_plan.Columns()) {
      // This is for the output schema of this TableFunction
      return_types.emplace_back(column.Type());
      names.emplace_back(column.Name());

      // This is for the PreparedStatementData
      fn_data->plan_data->types.emplace_back(column.Type());
      fn_data->plan_data->names.emplace_back(column.Name());
    }

    return fn_data;
  }

  // ------------------------------
  // Table function kernels

  static void
  TableFnTranslateMohair( ClientContext&      context
                         ,TableFunctionInput& data_p
                         ,DataChunk&          output) {
    auto &fn_data = (FnDataSubstraitTranslation&) *(data_p.bind_data);
    if (fn_data.finished) { return; }

    if (not fn_data.physical_plan) {
      // Convert plan to engine plan
      fn_data.logical_plan = fn_data.translator->TranspilePlanMessage(*(fn_data.sys_plan->duck_plan));

      // Convert engine plan to execution plan
      fn_data.physical_plan = fn_data.translator->TranslateLogicalPlan(
        *(fn_data.logical_plan), fn_data.enable_optimizer
      );

      fn_data.finished = true;
    }

    output.SetCardinality(1);
    output.SetValue(0, 0, fn_data.physical_plan->ToString());
  }

  static void
  TableFnExecuteMohair( ClientContext&      context
                       ,TableFunctionInput& data_p
                       ,DataChunk&          output) {
    auto& fn_data = (FnDataSubstraitExecution&) *(data_p.bind_data);

    if (not fn_data.executor) {
      // Convert plan to engine plan
      fn_data.logical_plan = fn_data.translator->TranspilePlanMessage(*(fn_data.sys_plan->duck_plan));

      // Convert engine plan to execution plan
      fn_data.plan_data->plan = fn_data.translator->TranslateLogicalPlan(*(fn_data.logical_plan), false);

      // Initialize a plan executor
      fn_data.executor = make_uniq<DuckDBExecutor>(context, *(fn_data.plan_data));
    }

    if (!fn_data.result) { fn_data.result = fn_data.executor->Execute(); }

    auto result_chunk = fn_data.result->Fetch();
    if (result_chunk) { output.Move(*result_chunk); }
  }



  // ------------------------------
  // Initializers for Table Functions that implement extension logic

  //! Create a TableFunction, "translate_mohair", then register it with the catalog
  void InitializeTranslateMohair(Connection &con) {
    TableFunction tablefn_mohair(
       "explain_mohair"
      ,{ LogicalType::BLOB }
      ,TableFnTranslateMohair
      ,BindingFnTranslateMohair
    );

    CreateTableFunctionInfo fninfo_mohair(tablefn_mohair);

    auto &catalog = Catalog::GetSystemCatalog(*(con.context));
    catalog.CreateTableFunction(*(con.context), fninfo_mohair);
  }

  //! Create a TableFunction, "execute_mohair", then register it with the catalog
  void InitializeExecuteMohair(Connection &con) {
    TableFunction tablefn_mohair(
       "execute_mohair"
      ,{ LogicalType::BLOB }
      ,TableFnExecuteMohair
      ,BindingFnExecuteMohair
    );

    CreateTableFunctionInfo fninfo_mohair(tablefn_mohair);

    auto &catalog = Catalog::GetSystemCatalog(*(con.context));
    catalog.CreateTableFunction(*(con.context), fninfo_mohair);
  }

  //! Create a TableFunction, "scan_sky_result", then register it with the catalog
  // TODO
  void InitializeSkyScan(Connection &con) {
    TableFunction tablefn_skyscan(
       "scan_sky_result"
      ,{ LogicalType::BLOB }
      ,TableFnExecuteMohair
      ,BindingFnExecuteMohair
    );

    CreateTableFunctionInfo fninfo_mohair(tablefn_mohair);

    auto &catalog = Catalog::GetSystemCatalog(*(con.context));
    catalog.CreateTableFunction(*(con.context), fninfo_mohair);
  }

  //! Logic for loading this extension
  void MohairExtension::Load(DuckDB& db) {
    Connection con(db);

    con.BeginTransaction();

    InitializeTranslateMohair(con);
    InitializeExecuteMohair(con);

    con.Commit();
  }

  std::string MohairExtension::Name() { return "mohair"; }

} // namespace duckdb


extern "C" {

  DUCKDB_EXTENSION_API
  void substrait_init(duckdb::DatabaseInstance& db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::MohairExtension>();
  }

  DUCKDB_EXTENSION_API
  const char* substrait_version() {
    return duckdb::DuckDB::LibraryVersion();
  }

}
