//---------------------------------------------------------------------
// <copyright file="edm_model_writer.cpp" company="Microsoft">
//      Copyright (C) Microsoft Corporation. All rights reserved. See License.txt in the project root for license information.
// </copyright>
//---------------------------------------------------------------------

#include "odata/edm/edm_model_writer.h"

//nd change
#include <iconv.h>
#include <string>
#include <iostream>
#include <cstring>

namespace odata {
    namespace edm {
        // nd change
        std::string xconvertToUTF8(const std::string &input, const char *srcEncoding) {
            iconv_t cd = iconv_open("UTF-8", srcEncoding);
            if (cd == (iconv_t) -1) {
                throw std::runtime_error("iconv_open failed to initialize");
            }

            // Allocate buffers for input and output
            size_t inBytes = input.size();
            size_t outBytes = input.size() * 4; // UTF-8 can take up to 4 bytes per character
            char *inBuffer = const_cast<char *>(input.data());
            char *outBuffer = new char[outBytes];
            char *outPtr = outBuffer;

            memset(outBuffer, 0, outBytes); // Clear output buffer

            // Perform encoding conversion
            size_t result = iconv(cd, &inBuffer, &inBytes, &outPtr, &outBytes);
            if (result == (size_t) -1) {
                iconv_close(cd);
                delete[] outBuffer;
                throw std::runtime_error("iconv conversion failed");
            }

            // Create a UTF-8 encoded string from the output buffer
            std::string utf8String(outBuffer);
            delete[] outBuffer;

            // Clean up
            iconv_close(cd);
            return utf8String;
        }


        void edm_model_writer::write_model(std::shared_ptr<edm_model> model) {
            // nd change
            // auto nsuri= xconvertToUTF8("http://docs.oasis-open.org/odata/ns/edmx","ISO-8859-1");
            // write_start_element_with_prefix(U("edmx"), U("Edmx"), nsuri);
            //
            // //write_attribute_string(U("xmlns"), U("edmx"), U(""), nsuri);
            // write_attribute_string(U(""), U("Version"), U(""), model->get_version());
            // //
            // write_start_element_with_prefix(U("edmx"), U("DataServices"));
            // for(auto schema_iter = model->get_schema().cbegin(); schema_iter != model->get_schema().cend(); schema_iter++)
            // {
            //     write_schema(*schema_iter);
            // }

            write_start_element_with_prefix(U("edmx"), U("Edmx"));
            write_attribute_string(U("xmlns"), U("edmx"), U(""), U("http://docs.oasis-open.org/odata/ns/edmx"));
            write_attribute_string(U(""), U("Version"), U(""), model->get_version());

            write_start_element_with_prefix(U("edmx"), U("DataServices"));
            for (auto schema_iter = model->get_schema().cbegin(); schema_iter != model->get_schema().cend(); schema_iter++) {
                write_schema(*schema_iter);
            }

            write_end_element();
            write_end_element();

            finalize();
        }

        void edm_model_writer::write_schema(std::shared_ptr<edm_schema> schema) {
            // nd change
            // write_start_element(U("Schema"));
            // write_attribute_string(U(""), U("xmlns"), U(""), std::string("http://docs.oasis-open.org/odata/ns/edm"));
            // write_attribute_string(U(""), U("Namespace"), U(""), schema->get_name());
            write_start_element(U("Schema"));
            write_attribute_string(U(""), U("xmlns"), U(""), U("http://docs.oasis-open.org/odata/ns/edm"));
            write_attribute_string(U(""), U("Namespace"), U(""), schema->get_name());

            for (auto iter = schema->get_enum_types().cbegin(); iter != schema->get_enum_types().cend(); iter++) {
                write_enum_type(iter->second);
            }

            for (auto iter = schema->get_complex_types().cbegin(); iter != schema->get_complex_types().cend(); iter++) {
                write_complex_type(iter->second);
            }

            for (auto iter = schema->get_entity_types().cbegin(); iter != schema->get_entity_types().cend(); iter++) {
                write_entity_type(iter->second);
            }

            for (auto iter = schema->get_operation_types().cbegin(); iter != schema->get_operation_types().cend(); iter++) {
                write_operation(iter->second);
            }

            for (auto iter = schema->get_containers().cbegin(); iter != schema->get_containers().cend(); iter++) {
                write_entity_container(iter->second);
            }

            write_end_element();
        }

        void edm_model_writer::write_enum_type(std::shared_ptr<edm_enum_type> enum_type) {
            write_start_element(U("EnumType"));

            write_attribute_string(U(""), U("Name"), U(""), enum_type->get_name());

            // nd change
            if (enum_type->is_flag())
                write_attribute_string(U(""), U("IsFlags"), U(""), "true");

            for (auto iter = enum_type->get_enum_members().cbegin(); iter != enum_type->get_enum_members().cend(); iter++) {
                write_start_element(U("Member"));
                write_attribute_string(U(""), U("Name"), U(""), (*iter)->get_enum_member_name());

                ::odata::utility::string_t str;
                ::odata::utility::stringstream_t ss;
                ss << (*iter)->get_enum_member_value();
                ss >> str;
                write_attribute_string(U(""), U("Value"), U(""), str);
                write_end_element();
            }

            write_end_element();
        }

        void edm_model_writer::write_complex_type(std::shared_ptr<edm_complex_type> complex_type) {
            write_start_element(U("ComplexType"));
            write_attribute_string(U(""), U("Name"), U(""), complex_type->get_name());

            for (auto iter = complex_type->begin(); iter != complex_type->end(); iter++) {
                write_start_element(U("Property"));
                write_attribute_string(U(""), U("Name"), U(""), iter->second->get_name());
                write_attribute_string(U(""), U("Type"), U(""), iter->second->get_property_type()->get_name());
                auto prp = iter->second;
                if (!prp->is_nullable())
                    write_attribute_string(U(""), U("Nullable"), U(""), "false");
                if (!prp->is_unicode())
                    write_attribute_string(U(""), U("Unicode"), U(""), "false");
                if (prp->get_max_length() != undefined_value)
                    write_attribute_string(U(""), U("MaxLength"), U(""), std::to_string(prp->get_max_length()));
                if (prp->get_precision() != undefined_value)
                    write_attribute_string(U(""), U("Precision"), U(""), std::to_string(prp->get_precision()));
                if (prp->get_scale() != 0)
                    write_attribute_string(U(""), U("Scale"), U(""), std::to_string(prp->get_scale()));
                if (!prp->default_value().empty())
                    write_attribute_string(U(""), U("DefaultValue"), U(""), prp->default_value());

                write_end_element();
            }

            write_end_element();
        }

        void edm_model_writer::write_entity_type(std::shared_ptr<edm_entity_type> entity_type) {
            write_start_element(U("EntityType"));
            write_attribute_string(U(""), U("Name"), U(""), entity_type->get_name());

            write_start_element(U("Key"));


            const auto kkey = entity_type->key();
            for (auto iter = kkey.cbegin(); iter != kkey.cend(); iter++) {
                write_start_element(U("PropertyRef"));
                write_attribute_string(U(""), U("Name"), U(""), *iter);
                write_end_element();
            }

            write_end_element();


            std::stack<std::shared_ptr<std::pair<const std::string, const std::shared_ptr<edm::edm_property_type>> > > entity_nav;
            std::stack<std::shared_ptr<std::pair<const std::string, const std::shared_ptr<edm::edm_property_type>> > > entity_prp;

            for (auto iter = entity_type->begin(); iter != entity_type->end(); iter++) {
                if (std::find(kkey.begin(), kkey.end(), iter->first) == kkey.end()) {
                    if (iter->second->get_property_type()->get_type_kind() == edm::edm_type_kind_t::Navigation) {
                        entity_nav.push(
                            std::make_shared<std::pair<const std::string, const std::shared_ptr<edm::edm_property_type>> >(
                                iter->first, iter->second
                            )
                        );
                        continue;
                    } else {
                        entity_prp.push(
                            std::make_shared<std::pair<const std::string, const std::shared_ptr<edm::edm_property_type>> >(
                                iter->first, iter->second
                            )
                        );
                        continue;
                    }
                }
                write_start_element(U("Property"));
                write_attribute_string(U(""), U("Name"), U(""), iter->second->get_name());
                write_attribute_string(U(""), U("Type"), U(""), iter->second->get_property_type()->get_name());
                auto prp = iter->second;
                if (!prp->is_nullable())
                    write_attribute_string(U(""), U("Nullable"), U(""), "false");
                if (!prp->is_unicode())
                    write_attribute_string(U(""), U("Unicode"), U(""), "false");
                if (prp->get_max_length() != undefined_value)
                    write_attribute_string(U(""), U("MaxLength"), U(""), std::to_string(prp->get_max_length()));
                if (prp->get_precision() != undefined_value)
                    write_attribute_string(U(""), U("Precision"), U(""), std::to_string(prp->get_precision()));
                if (prp->get_scale() != 0)
                    write_attribute_string(U(""), U("Scale"), U(""), std::to_string(prp->get_scale()));
                if (!prp->default_value().empty())
                    write_attribute_string(U(""), U("DefaultValue"), U(""), prp->default_value());

                write_end_element();
            }

            while (!entity_prp.empty()) {
                const auto iter = entity_prp.top();
                entity_prp.pop();
                write_start_element(U("Property"));
                write_attribute_string(U(""), U("Name"), U(""), iter->second->get_name());
                write_attribute_string(U(""), U("Type"), U(""), iter->second->get_property_type()->get_name());
                auto prp = iter->second;
                if (!prp->is_nullable())
                    write_attribute_string(U(""), U("Nullable"), U(""), "false");
                if (!prp->is_unicode())
                    write_attribute_string(U(""), U("Unicode"), U(""), "false");
                if (prp->get_max_length() != undefined_value)
                    write_attribute_string(U(""), U("MaxLength"), U(""), std::to_string(prp->get_max_length()));
                if (prp->get_precision() != undefined_value)
                    write_attribute_string(U(""), U("Precision"), U(""), std::to_string(prp->get_precision()));
                if (prp->get_scale() != 0)
                    write_attribute_string(U(""), U("Scale"), U(""), std::to_string(prp->get_scale()));
                if (!prp->default_value().empty())
                    write_attribute_string(U(""), U("DefaultValue"), U(""), prp->default_value());

                write_end_element();
            }

            while (!entity_nav.empty()) {
                const auto iter = entity_nav.top();
                entity_nav.pop();

                write_start_element(U("NavigationProperty"));
                write_attribute_string(U(""), U("Name"), U(""), iter->second->get_name());
                write_attribute_string(U(""), U("Type"), U(""), iter->second->get_property_type()->get_name());
                auto prp = iter->second;
                if (!prp->is_nullable())
                    write_attribute_string(U(""), U("Nullable"), U(""), "false");
                auto ptype = static_cast<edm::edm_navigation_type *>(prp->get_property_type().get());
                if (ptype->is_contained())
                    write_attribute_string(U(""), U("ContainsTarget"), U(""), "true");

                write_end_element();
            }
            write_end_element();
        }

        void edm_model_writer::write_operation(std::shared_ptr<edm_operation_type> operation) {
            write_start_element(operation->is_function() ? U("Function") : U("Action"));
            write_attribute_string(U(""), U("Name"), U(""), operation->get_name());
            if (operation->is_bound())
                write_attribute_string(U(""), U("IsBound"), U(""), U("true"));
            if (operation->is_composable())
                write_attribute_string(U(""), U("IsComposable"), U(""), U("true"));
            if (!operation->get_path().empty())
                write_attribute_string(U(""), U("EntitySetPath"), U(""), operation->get_path());


            for (auto iter = operation->get_operation_parameters().cbegin(); iter != operation->get_operation_parameters().cend(); iter++) {
                write_start_element(U("Parameter"));
                write_attribute_string(U(""), U("Name"), U(""), (*iter)->get_param_name());
                write_attribute_string(U(""), U("Type"), U(""), (*iter)->get_param_type()->get_name());
                if (!(*iter)->is_nullable())
                     write_attribute_string(U(""), U("Nullable"), U(""), "false");
                auto prp = (*iter)->get_param_type();
                switch (prp->get_type_kind())
                {
                    case edm::edm_type_kind_t::Primitive:{
                        auto primitive_type = std::dynamic_pointer_cast<edm::edm_primitive_type>(prp);
                        write_end_element();
                    }
                    break;
                    case edm::edm_type_kind_t::Enum:{
                        auto enum_type = std::dynamic_pointer_cast<edm::edm_enum_type>(prp);
                        write_end_element();
                    }
                    break;
                    case edm::edm_type_kind_t::Collection:{
                        auto collection_type = std::dynamic_pointer_cast<edm::edm_collection_type>(prp);
                        write_end_element();
                    }
                    break;
                    case edm::edm_type_kind_t::Complex:{
                        auto structured_type = std::dynamic_pointer_cast<edm::edm_complex_type>(prp);
                        write_end_element();
                    }
                    break;
                    case edm::edm_type_kind_t::Entity:{
                        auto entity_type = std::dynamic_pointer_cast<edm::edm_entity_type>(prp);
                        write_end_element();
                        // structured_type->is_open_type()
                    }
                    break;
                    default:{
                        //throw new odata_service_exception(U("Invalid edm type."));
                    }
                    break;
                }
                //
                // if (!prp->is_nullable())
                //     write_attribute_string(U(""), U("Nullable"), U(""), "false");
                // if (!prp->is_unicode())
                //     write_attribute_string(U(""), U("Unicode"), U(""), "false");
                // if (prp->get_max_length() != undefined_value)
                //     write_attribute_string(U(""), U("MaxLength"), U(""), std::to_string(prp->get_max_length()));
                // if (prp->get_precision() != undefined_value)
                //     write_attribute_string(U(""), U("Precision"), U(""), std::to_string(prp->get_precision()));
                // if (prp->get_scale() != 0)
                //     write_attribute_string(U(""), U("Scale"), U(""), std::to_string(prp->get_scale()));
                // if (!prp->default_value().empty())
                //     write_attribute_string(U(""), U("DefaultValue"), U(""), prp->default_value());



                //write_end_element();
            }

            if (operation->get_operation_return_type()) {
                write_start_element(U("ReturnType"));
                auto ptype=operation->get_operation_return_type();
                write_attribute_string(U(""), U("Type"), U(""), ptype->get_name());
                write_end_element();
            }

            write_end_element();
        }

        void edm_model_writer::write_entity_container(std::shared_ptr<edm_entity_container> entity_container) {
            write_start_element(U("EntityContainer"));
            write_attribute_string(U(""), U("Name"), U(""), entity_container->get_name());

            for (auto iter = entity_container->begin(); iter != entity_container->end(); iter++) {
                write_entity_set(iter->second);
            }

            for (auto iter = entity_container->get_singletons().cbegin(); iter != entity_container->get_singletons().cend(); iter++) {
                write_singleton(iter->second);
            }

            for (auto iter = entity_container->get_operation_imports().cbegin(); iter != entity_container->get_operation_imports().cend(); iter++) {
                write_operation_import(iter->second);
            }

            write_end_element();
        }

        void edm_model_writer::write_entity_set(std::shared_ptr<edm_entity_set> entity_set) {
            write_start_element(U("EntitySet"));
            write_attribute_string(U(""), U("Name"), U(""), entity_set->get_name());
            write_attribute_string(U(""), U("EntityType"), U(""), entity_set->get_entity_type_name());

            for (auto iter = entity_set->get_navigation_sources().cbegin(); iter != entity_set->get_navigation_sources().cend(); iter++) {
                write_start_element(U("NavigationPropertyBinding"));
                write_attribute_string(U(""), U("Path"), U(""), iter->first);
                write_attribute_string(U(""), U("Target"), U(""), iter->second);
                write_end_element();
            }

            write_end_element();
        }

        void edm_model_writer::write_singleton(std::shared_ptr<edm_singleton> singleton) {
            write_start_element(U("Singleton"));
            write_attribute_string(U(""), U("Name"), U(""), singleton->get_name());
            write_attribute_string(U(""), U("EntityType"), U(""), singleton->get_entity_type_name());

            for (auto iter = singleton->get_navigation_sources().cbegin(); iter != singleton->get_navigation_sources().cend(); iter++) {
                write_start_element(U("NavigationPropertyBinding"));
                write_attribute_string(U(""), U("Path"), U(""), iter->first);
                write_attribute_string(U(""), U("Target"), U(""), iter->second);
                write_end_element();
            }

            write_end_element();
        }

        void edm_model_writer::write_operation_import(std::shared_ptr<edm_operation_import> operation_import) {
            write_start_element(operation_import->get_operation_import_kind() == OperationImportKind::FunctionImport ? U("FunctionImport") : U("ActionImport"));
            write_attribute_string(U(""), U("Name"), U(""), operation_import->get_name());
            write_attribute_string(U(""), operation_import->get_operation_import_kind() == OperationImportKind::FunctionImport ? U("Function"):U("Action"), U(""), operation_import->get_operation_name());
            if (!operation_import->get_entity_set_name().empty())
                write_attribute_string(U(""), U("EntitySet"), U(""), operation_import->get_entity_set_name());

            write_end_element();
        }
    }
}
