#include "core/latview.h"

#include <cmath>

#include "core/dllmath.h"

namespace avas {

namespace {

int add_slot(std::vector<kern::RfSlotV>& slots, int cav, double freq, bool hasAdd, double add) {
    if (cav < 0) return -1;
    for (size_t k = 0; k < slots.size(); ++k) {
        const kern::RfSlotV& s = slots[k];
        // bit-identical keys only (the phase expression depends on the exact values)
        if (s.cav == cav && s.freq == freq && s.hasAdd == (hasAdd ? 1 : 0) &&
            (!hasAdd || (std::signbit(s.add) == std::signbit(add) && s.add == add)))
            return static_cast<int>(k);
    }
    slots.push_back(kern::RfSlotV{cav, hasAdd ? 1 : 0, freq, add});
    return static_cast<int>(slots.size()) - 1;
}

kern::ElemV elem_view(const Element& e) {
    kern::ElemV v{};
    v.kind = static_cast<int>(e.kind);
    v.fieldType = e.fieldType;
    v.mapIndex = e.mapIndex;
    v.inSuperposeOut = e.inSuperposeOut;
    v.rotMode = e.rotMode;
    v.zStart = e.zStart;
    v.length = e.length;
    v.radius = e.radius;
    v.strength = e.strength;
    v.ke = e.ke;
    v.kb = e.kb;
    v.keErr = e.keErr;
    v.kbErr = e.kbErr;
    v.phaseErr = e.phaseErr;
    v.errDx = e.errDx;
    v.errDy = e.errDy;
    v.errDz = e.errDz;
    dll_sincos(e.errRotX, v.esx, v.ecx);
    dll_sincos(e.errRotY, v.esy, v.ecy);
    dll_sincos(e.errRotZ, v.esz, v.ecz);
    v.rfPlain = v.rfErr = -1;
    v.rfq0 = v.rfqN = 0;
    return v;
}

void add_subs(const Element& e, kern::ElemV& v, std::vector<kern::SubV>& subs, std::vector<kern::RfSlotV>& slots) {
    v.sub0 = static_cast<int>(subs.size());
    v.nsub = static_cast<int>(e.subZStart.size());
    for (size_t q = 0; q < e.subZStart.size(); ++q) {
        kern::SubV s{};
        s.type = q < e.subType.size() ? e.subType[q] : 0;
        s.map = q < e.subMap.size() ? e.subMap[q] : -1;
        s.ke = q < e.subKe.size() ? e.subKe[q] : 0.0;
        s.kb = q < e.subKb.size() ? e.subKb[q] : 0.0;
        s.zStart = e.subZStart[q];
        s.radius = q < e.subRadius.size() ? e.subRadius[q] : 0.0;
        s.length = q < e.subLength.size() ? e.subLength[q] : 0.0;
        if (q < e.subPlace.size())
            for (int j = 0; j < 6; ++j) s.place[j] = e.subPlace[q][j];
        dll_sincos(s.place[3], s.s3, s.c3);
        dll_sincos(s.place[4], s.s4, s.c4);
        dll_sincos(s.place[5], s.s5, s.c5);
        if (q < e.subErr.size())
            for (int j = 0; j < 12; ++j) s.err[j] = e.subErr[q][j];
        s.hasErr = s.err[0] == 1.0 ? 1 : 0;
        s.es4 = 0.0; s.ec4 = 1.0; s.es5 = 0.0; s.ec5 = 1.0; s.es6 = 0.0; s.ec6 = 1.0; s.esin6 = 0.0;
        if (s.hasErr) {
            dll_sincos(s.err[6], s.es6, s.ec6);
            dll_sincos(s.err[5], s.es5, s.ec5);
            dll_sincos(s.err[4], s.es4, s.ec4);
            s.esin6 = dll_sin(s.err[6]);
        }
        s.rfErr = add_slot(slots, e.cavIndex, e.frequency, true, s.err[9]);
        subs.push_back(s);
    }
}

}  // namespace

kern::PartV beam_view(Beam& b) {
    kern::PartV v{};
    v.x = b.x.data(); v.ux = b.ux.data(); v.y = b.y.data(); v.uy = b.uy.data(); v.z = b.z.data(); v.uz = b.uz.data();
    v.ex = b.ex.data(); v.ey = b.ey.data(); v.ez = b.ez.data(); v.bx = b.bx.data(); v.by = b.by.data(); v.bz = b.bz.data();
    v.scx = b.scx.data(); v.scy = b.scy.data(); v.scz = b.scz.data();
    for (int j = 0; j < 6; ++j) v.rel[j] = b.rel[j].data();
    v.elemIndex = b.elemIndex.data(); v.lost = b.lost.data(); v.nextPlane = b.nextPlane.data();
    v.boundaryMode = b.boundaryMode.data(); v.geomIndex = b.geomIndex.data(); v.spOutCount = b.spOutCount.data();
    v.weight = b.weight.data(); v.qOverM = b.qOverM.data(); v.charge = b.charge.data();
    v.spOutZ0 = b.spOutZ0.data(); v.spOutZ1 = b.spOutZ1.data();
    v.t = b.t.data();
    v.dt = b.dt.data();
    v.sync = b.n;
    return v;
}

void LatHost::build(const Lattice& lat, const std::vector<char>& errUse, const std::vector<char>& hetErr,
                    const std::vector<Element>& spEff) {
    elems.clear();
    eff.clear();
    subs.clear();
    maps.clear();
    slots.clear();
    spOut.clear();
    geoms.clear();
    rfqCells.clear();
    errOn = !errUse.empty();
    hasEff = !spEff.empty();
    const size_t ne = lat.elements.size();
    for (size_t k = 0; k < ne; ++k) {
        const Element& e = lat.elements[k];
        kern::ElemV v = elem_view(e);
        v.errUse = errUse.size() > k && errUse[k] ? 1 : 0;
        v.hetErr = hetErr.size() > k && hetErr[k] ? 1 : 0;
        if (e.kind == ElemKind::Field) {
            v.rfPlain = add_slot(slots, e.cavIndex, e.frequency, false, 0.0);
            v.rfErr = add_slot(slots, e.cavIndex, e.frequency, true, e.phaseErr);
        } else if (e.kind == ElemKind::Rfq) {
            v.rfPlain = v.rfErr = add_slot(slots, e.cavIndex, e.frequency, false, 0.0);
            v.rfq0 = static_cast<int>(rfqCells.size() / 14);
            v.rfqN = static_cast<int>(e.rfqCells.size());
            for (const auto& c : e.rfqCells) {
                for (int j = 0; j < 14; ++j) rfqCells.push_back(j < 2 ? c[j] - e.zStart : c[j]);
            }
        }
        add_subs(e, v, subs, slots);
        elems.push_back(v);
    }
    if (hasEff) {
        for (size_t k = 0; k < spEff.size(); ++k) {
            const Element& e = spEff[k];
            kern::ElemV v = elem_view(e);
            if (e.kind == ElemKind::Field) {
                v.rfPlain = add_slot(slots, e.cavIndex, e.frequency, false, 0.0);
                v.rfErr = add_slot(slots, e.cavIndex, e.frequency, true, e.phaseErr);
            }
            add_subs(e, v, subs, slots);
            eff.push_back(v);
        }
    }
    for (const FieldMap& m : lat.fm.maps) maps.push_back(m.view());
    zStart = lat.elemZStart;
    zEnd = lat.elemZEnd;
    for (const auto& r : lat.spOut) {
        kern::SpOutV s{};
        for (int j = 0; j < 7; ++j) s.row[j] = r[j];
        dll_sincos(r[3], s.s3, s.c3);
        dll_sincos(r[5], s.s5, s.c5);
        dll_sincos(r[4], s.s4, s.c4);
        spOut.push_back(s);
    }
    cavTime = lat.fm.cavTime;
    cavPhase = lat.fm.cavPhase;
    // one geometry per segment, then the sub-shapes of the composite geometries (shape 0)
    auto geom_v = [](const Geometry& g, int sub0) {
        kern::GeomV v{};
        v.shape = g.shape;
        v.npar = static_cast<int>(g.par.size());
        v.sub0 = g.shape == 0 ? sub0 + g.sub0 : 0;
        v.nsub = g.shape == 0 ? g.nsub : 0;
        v.zStart = g.zStart;
        for (size_t j = 0; j < 6 && j < g.par.size(); ++j) v.par[j] = g.par[j];
        for (int j = 0; j < 6; ++j) v.place[j] = g.place[j];
        dll_sincos(g.place[3], v.s3, v.c3);
        dll_sincos(g.place[5], v.s5, v.c5);
        dll_sincos(g.place[4], v.s4, v.c4);
        return v;
    };
    const int nGeom = static_cast<int>(lat.bnd.geoms.size());
    for (const Geometry& g : lat.bnd.geoms) geoms.push_back(geom_v(g, nGeom));
    for (const Geometry& g : lat.bnd.subs) geoms.push_back(geom_v(g, nGeom));
    segStart = lat.bnd.segStart;
    segEnd = lat.bnd.segEnd;
    bndZ.clear();
    bndR1.clear();
    bndR2.clear();
    for (const auto& r : lat.bnd.rows) {
        bndZ.push_back(r[0]);
        bndR1.push_back(static_cast<int>(std::lround(r[1])));
        bndR2.push_back(static_cast<int>(std::lround(r[2])));
    }
}

bool LatHost::sync_dynamic(const Lattice& lat) {
    bool changed = false;
    for (size_t k = 0; k < elems.size() && k < lat.elements.size(); ++k) {
        if (elems[k].rotMode != lat.elements[k].rotMode) {
            elems[k].rotMode = lat.elements[k].rotMode;
            changed = true;
        }
    }
    return changed;
}

kern::LatV LatHost::view(const Lattice& lat) const {
    kern::LatV v{};
    v.elems = elems.data();
    v.eff = hasEff ? eff.data() : nullptr;
    v.subs = subs.data();
    v.maps = maps.data();
    v.elemZStart = zStart.data();
    v.elemZEnd = zEnd.data();
    v.spOut = spOut.data();
    v.cavTime = cavTime.data();
    v.cavPhase = cavPhase.data();
    v.rfSlots = slots.data();
    v.rfqCells = rfqCells.empty() ? nullptr : rfqCells.data();
    v.nElem = static_cast<int>(elems.size());
    v.errOn = errOn ? 1 : 0;
    v.spOutCur = lat.spOutCur;
    v.spOutCount = lat.spOutCount;
    v.spOutSize = static_cast<int>(lat.spOut.size());
    v.totalLength = lat.total_length();
    v.spExit = lat.sp_exit();
    v.geoms = geoms.data();
    v.segStart = segStart.data();
    v.segEnd = segEnd.data();
    v.nSeg = static_cast<int>(segEnd.size());
    v.bndZ = bndZ.data();
    v.bndR1 = bndR1.data();
    v.bndR2 = bndR2.data();
    v.nBnd = static_cast<int>(bndZ.size());
    return v;
}

void LatHost::rf_table(double t, std::vector<double>& out) const {
    out.resize(2 * slots.size());
    for (size_t q = 0; q < slots.size(); ++q) {
        const kern::RfSlotV& s = slots[q];
        const double ph = s.hasAdd ? (t - cavTime[s.cav]) * s.freq * k::two_pi + (cavPhase[s.cav] + s.add) * k::deg_to_rad
                                   : (t - cavTime[s.cav]) * s.freq * k::two_pi + cavPhase[s.cav] * k::deg_to_rad;
        out[2 * q] = dll_cos(ph);
        out[2 * q + 1] = dll_cos(ph - k::half_pi);
    }
}

}  // namespace avas
