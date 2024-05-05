#pragma once

#include <iostream>
#include <vector>
#include <string_view>
#include <cstdint>
#include <thread>
#include <fstream>
#include <array>
#include <concepts>

#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <nfd.h>

namespace ns {
    template<typename T, const char* in_name>
    struct Value {
        static constexpr auto name { in_name };
        T value;
    };

    namespace ValueNames {
        static constexpr char freq[] { "freq" };
        static constexpr char unix_timestamp[] { "unix_timestamp" };
        // RAW
        static constexpr char raw_real_data[] { "raw_real_data" };
        static constexpr char raw_imag_data[] { "raw_imag_data" };

        // CALCULATED
        static constexpr char calculated_magnitude[] { "calculated_magnitude" };
        static constexpr char calculated_phase[] { "calculated_phase" };

        // CALIBRATION
        static constexpr char gain_factor[] { "gain_factor" };
        static constexpr char sys_phase[] { "sys_phase" };

        // CORRECTED
        static constexpr char corrected_impedance[] { "corrected_impedance" };
        static constexpr char corrected_phase[] { "corrected_phase" };
        static constexpr char corrected_resistance[] { "corrected_resistance" };
        static constexpr char corrected_reactance[] { "corrected_reactance" };
    }

    using Freq = Value<float, ValueNames::freq>;
    using UnixTimestamp = Value<double, ValueNames::unix_timestamp>;

    template<std::floating_point T>
    using RawRealData = Value<T, ValueNames::raw_real_data>;

    template<std::floating_point T>
    using RawImagData = Value<T, ValueNames::raw_imag_data>;

    template<std::floating_point T>
    using CalculatedMagnitude = Value<T, ValueNames::calculated_magnitude>;

    template<std::floating_point T>
    using CalculatedPhase = Value<T, ValueNames::calculated_phase>;

    template<std::floating_point T>
    using GainFactor = Value<T, ValueNames::gain_factor>;

    template<std::floating_point T>
    using SysPhase = Value<T, ValueNames::sys_phase>;

    template<std::floating_point T>
    using CorrectedImpedance = Value<T, ValueNames::corrected_impedance>;

    template<std::floating_point T>
    using CorrectedPhase = Value<T, ValueNames::corrected_phase>;

    template<std::floating_point T>
    using CorrectedResistance = Value<T, ValueNames::corrected_resistance>;

    template<std::floating_point T>
    using CorrectedReactance = Value<T, ValueNames::corrected_reactance>;
}

namespace ns {
    template<typename T_Value, typename U_Value, typename V_Value>
    struct DoublePoint2D {
        T_Value x;
        U_Value y1;
        V_Value y2;
    };

    template<std::floating_point T, const char* x_name>
    using RawDataPoint = DoublePoint2D<Value<T, x_name>, RawRealData<T>, RawImagData<T>>;

    template<std::floating_point T, const char* x_name>
    using CalculatedPoint = DoublePoint2D<Value<T, x_name>, CalculatedMagnitude<T>, CalculatedPhase<T>>;

    template<std::floating_point T, const char* x_name>
    using CalibrationPoint = DoublePoint2D<Value<T, x_name>, GainFactor<T>, SysPhase<T>>;

    template<std::floating_point T, const char* x_name>
    using CorrectedGonPoint = DoublePoint2D<Value<T, x_name>, CorrectedImpedance<T>, CorrectedPhase<T>>;

    template<std::floating_point T, const char* x_name>
    using CorrectedAlgPoint = DoublePoint2D<Value<T, x_name>, CorrectedResistance<T>, CorrectedReactance<T>>;

    template<typename T, typename U, typename V>
    void to_json(json& j, const DoublePoint2D<T, U, V>& p) {
        j = json {
            { p.x.name, p.x.value },
            { p.y1.name, p.y1.value },
            { p.y2.name, p.y2.value },
        };
    }

    template<typename T, typename U, typename V>
    void from_json(const json& j, DoublePoint2D<T, U, V>& p) {
        j.at(p.x.name).get_to(p.value);
        j.at(p.y1.name).get_to(p.y1.value);
        j.at(p.y2.name).get_to(p.y2.value);
    }
}

namespace ns {
    template<typename T_Point, const char* in_name>
    struct Graph2D {
        static constexpr auto name { in_name };
        std::vector<T_Point> points;
    };

    namespace GraphNames {
        static constexpr char raw_data[] { "raw_data" };
        static constexpr char calculated_data[] { "calculated_data" };
        static constexpr char calibration_data[] { "calibration_data" };
        static constexpr char corrected_gon_data[] { "corrected_gon_data" };
        static constexpr char corrected_alg_data[] { "corrected_alg_data" };
    }

    template<std::floating_point T, const char* x_name>
    using RawDataGraph2D = Graph2D<RawDataPoint<T, x_name>, GraphNames::raw_data>;

    template<std::floating_point T, const char* x_name>
    using CalculatedGraph2D = Graph2D<CalculatedPoint<T, x_name>, GraphNames::calculated_data>;

    template<std::floating_point T, const char* x_name>
    using CalibrationGraph2D = Graph2D<CalibrationPoint<T, x_name>, GraphNames::calibration_data>;

    template<std::floating_point T, const char* x_name>
    using CorrectedGonGraph2D = Graph2D<CorrectedGonPoint<T, x_name>, GraphNames::corrected_gon_data>;

    template<std::floating_point T, const char* x_name>
    using CorrectedAlgGraph2D = Graph2D<CorrectedAlgPoint<T, x_name>, GraphNames::corrected_alg_data>;

    template<typename T_Point, const char* in_name>
    void to_json(json& j, const Graph2D<T_Point, in_name>& p) {
        j = json {
            { p.name, p.points },
        };
    }

    template<typename T_Point, const char* in_name>
    void from_json(const json& j, Graph2D<T_Point, in_name>& p) {
        j.at(p.name).get_to(p.points);
    }
}

namespace ns {
    template<const char* graph_name, typename T_Graph2D>
    struct Graph2D_File {
        static constexpr auto name { graph_name };
        T_Graph2D graph_2D;
    };

    template<const char* graph_name, typename T_Graph2D>
    void to_json(json& j, const Graph2D_File<graph_name, T_Graph2D>& p) {
        j = json {
            { p.name, p.graph_2D },
        };
    }

    template<const char* graph_name, typename T_Graph2D>
    void from_json(const json& j, Graph2D_File<graph_name, T_Graph2D>& p) {
        j.at(p.name).get_to(p.graph_2D);
    }

    template<const char* graph_name, std::floating_point T, const char* x_name>
    using RawDataGraph2D_File = Graph2D_File<graph_name, RawDataGraph2D<T, x_name>>;

    template<const char* graph_name, std::floating_point T, const char* x_name>
    using CalculatedGraph2D_File = Graph2D_File<graph_name, CalculatedGraph2D<T, x_name>>;

    template<const char* graph_name, std::floating_point T, const char* x_name>
    using CalibrationGraph2D_File = Graph2D_File<graph_name, CalibrationGraph2D<T, x_name>>;

    template<const char* graph_name, std::floating_point T, const char* x_name>
    using CorrectedGonGraph2D_File = Graph2D_File<graph_name, CorrectedGonGraph2D<T, x_name>>;

    template<const char* graph_name, std::floating_point T, const char* x_name>
    using CorrectedAlgGraph2D_File = Graph2D_File<graph_name, CorrectedAlgGraph2D<T, x_name>>;
}

namespace ns {
    template<typename Z_Value, typename T_Graph2D>
    struct Point3D {
        Z_Value z;
        T_Graph2D graph_2D;
    };

    template<typename Z_Value, std::floating_point T, const char* x_name>
    using RawDataPoint3D = Point3D<Z_Value, RawDataGraph2D<T, x_name>>;

    template<typename Z_Value, std::floating_point T, const char* x_name>
    using CalculatedPoint3D = Point3D<Z_Value, CalculatedGraph2D<T, x_name>>;

    template<typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedGonPoint3D = Point3D<Z_Value, CorrectedGonGraph2D<T, x_name>>;

    template<typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedAlgPoint3D = Point3D<Z_Value, CorrectedAlgGraph2D<T, x_name>>;

    template<typename Z_Value, typename T_Graph2D>
    void to_json(json& j, const Point3D<Z_Value, T_Graph2D>& p) {
        j = json {
            { p.z.name, p.z.value },
            { p.graph_2D.name, p.graph_2D },
        };
    }

    template<typename Z_Value, typename T_Graph2D>
    void from_json(const json& j, Point3D<Z_Value, T_Graph2D>& p) {
        j.at(p.z.name).get_to(p.z.value);
        j.at(p.graph_2D.name).get_to(p.graph_2D);
    }
}

namespace ns {
    template<const char* graph_name, typename T_Point3D>
    struct Graph3D {
        static constexpr auto name { graph_name };
        std::vector<T_Point3D> points;
    };

    template<const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using RawDataGraph3D = Graph3D<graph_name, RawDataPoint3D<Z_Value, T, x_name>>;

    template<const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CalculatedGraph3D = Graph3D<graph_name, CalculatedPoint3D<Z_Value, T, x_name>>;

    template<const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedGonGraph3D = Graph3D<graph_name, CorrectedGonPoint3D<Z_Value, T, x_name>>;

    template<const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedAlgGraph3D = Graph3D<graph_name, CorrectedAlgPoint3D<Z_Value, T, x_name>>;

    template<const char* graph_name, typename T_Point3D>
    void to_json(json& j, const Graph3D<graph_name, T_Point3D>& p) {
        j = json {
            { p.name, p.points },
        };
    }

    template<const char* graph_name, typename T_Point3D>
    void from_json(const json& j, Graph3D<graph_name, T_Point3D>& p) {
        j.at(p.name).get_to(p.points);
    }
}

namespace ns {
    template<const char* graph_file_name, typename T_Graph3D>
    struct Graph3D_File {
        static constexpr auto name { graph_file_name };
        T_Graph3D graph_3D;
    };

    template<const char* graph_file_name, const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using RawDataGraph3D_File = Graph3D_File<graph_file_name, RawDataGraph3D<graph_name, Z_Value, T, x_name>>;

    template<const char* graph_file_name, const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CalculatedGraph3D_File = Graph3D_File<graph_file_name, CalculatedGraph3D<graph_name, Z_Value, T, x_name>>;

    template<const char* graph_file_name, const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedGonGraph3D_File = Graph3D_File<graph_file_name, CorrectedGonGraph3D<graph_name, Z_Value, T, x_name>>;

    template<const char* graph_file_name, const char* graph_name, typename Z_Value, std::floating_point T, const char* x_name>
    using CorrectedAlgGraph3D_File = Graph3D_File<graph_file_name, CorrectedAlgGraph3D<graph_name, Z_Value, T, x_name>>;

    namespace Graph3D_Names {
        static constexpr char zfreq_raw_data[] { "zfreq_raw_data" };
        static constexpr char zfreq_calculated[] { "zfreq_calculated" };
        static constexpr char zfreq_corrected_gon[] { "zfreq_corrected_gon" };
        static constexpr char zfreq_corrected_alg[] { "zfreq_corrected_alg" };
    }

    template<const char* graph_file_name, typename T_Graph3D>
    void to_json(json& j, const Graph3D_File<graph_file_name, T_Graph3D>& p) {
        j = json {
            { p.name, p.graph_3D },
        };
    }

    template<const char* graph_file_name, typename T_Graph3D>
    void from_json(const json& j, Graph3D_File<graph_file_name, T_Graph3D>& p) {
        j.at(p.name).get_to(p.graph_3D);
    }
}

namespace ns {
    template<typename T_Graph2D_File, std::floating_point T>
    void load_graph2D_file(const std::vector<T> xs, const std::vector<T> ys1, const std::vector<T> ys2, T_Graph2D_File& out_graph_file) {
        const size_t wished_size { std::min(std::min(xs.size(), ys1.size()), ys2.size()) };
        out_graph_file.graph_2D.points.reserve(wished_size);
        for(size_t i = 0; i < wished_size; i++) {
            out_graph_file.graph_2D.points.push_back(
                {
                    { xs[i] },
                    { ys1[i] },
                    { ys2[i] }
                }
            );
        }
    }

    template<typename T_Graph3D_File, typename T_Point>
    void load_graph3D_file(const std::vector<float> freq, const std::vector<double> time_points, const std::vector<T_Point> points, T_Graph3D_File& out_graph3D_file) {
        const size_t wished_size { freq.size() };
        out_graph3D_file.graph_3D.points.reserve(wished_size);
        for(size_t i = 0; i < wished_size; i++) {
            out_graph3D_file.graph_3D.points.push_back(
                {
                    freq[i],
                }
            );
            const size_t min { std::min(std::min(time_points.size(), points[i].raw.real_data.size()), points[i].raw.imag_data.size()) };
            out_graph3D_file.graph_3D.points[i].graph_2D.points.reserve(min);
            for(size_t j = 0; j < min; j++) {
                out_graph3D_file.graph_3D.points[i].graph_2D.points.push_back(
                    {
                        time_points[j],
                        points[i].raw.real_data[j],
                        points[i].raw.imag_data[j],
                    }
                );
            }
        }
    }

    template<typename T>
    void save_to_fs(const T obj) {
        nfdchar_t* outPath { nullptr };
        const std::array<nfdfilteritem_t, 1> filterItem {
            #ifdef _MSC_VER
                { L"Graph", L"json" }
            #else
                { "Graph", "json" }
            #endif
        };
        const nfdresult_t result { 
            NFD_SaveDialog(
                &outPath,
                filterItem.data(),
                static_cast<nfdfiltersize_t>(filterItem.size()),
                nullptr,
                #ifdef _MSC_VER
                    L"graph.json"
                #else
                    "graph.json"
                #endif
            )
        };
        if(result == NFD_OKAY) {
            const json j = obj;
            std::ofstream(std::filesystem::path(outPath)) << std::setw(4) << j;
            if(outPath != nullptr) {
                NFD_FreePath(outPath);
                outPath = nullptr;
            }
        } else if(result == NFD_CANCEL) {
            std::printf("User pressed cancel!\n");
            if(outPath != nullptr) {
                NFD_FreePath(outPath);
                outPath = nullptr;
            }
            return;
        } else {
            std::printf("Error: %s\n", NFD_GetError());
            if(outPath != nullptr) {
                NFD_FreePath(outPath);
                outPath = nullptr;
            }
            return;
        }
    }
}