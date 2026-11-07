// AVAS.dll-only tools (spec 80 3-5): sampling, meshing, dataprocess.
#pragma once
#include <string>

namespace avas {

int tool_sampling(const std::string& in, const std::string& out, int p3, int p4, int p5, int p6, int p7, int p8,
                  double p9, double ct);
int tool_meshing(const std::string& in, const std::string& out, int n, double mag);
int tool_dataprocess(const std::string& root, int i0, int i1, double g0, double g1, double mag, int useW);

}  // namespace avas
