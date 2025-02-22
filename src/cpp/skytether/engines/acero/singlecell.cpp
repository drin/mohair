
// >> Util functions

Result<shared_ptr<Array>> EmptyDoubles(int64_t val_count) {
    arrow::DoubleBuilder arr_builder;

    ARROW_RETURN_NOT_OK(arr_builder.AppendEmptyValues(val_count));

    return arr_builder.Finish();
}

Result<shared_ptr<Array>> CopyStrArray(shared_ptr<Array> src_array) {
    auto str_array = std::static_pointer_cast<StringArray>(src_array);

    arrow::StringBuilder array_builder;
    ARROW_RETURN_NOT_OK(array_builder.Resize(str_array->length()));

    for (const auto str_val_opt : *str_array) {
        array_builder.Append(str_val_opt.value_or(""));
    }

    return array_builder.Finish();
}
