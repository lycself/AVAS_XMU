// Study page (page id "scan"): parameter scans, stability charts, response & controllability and
// feedback, on the linear model and the engine (avas/gui/services/study.py, analysis.py).
import { useState } from "react";
import { Tabs } from "../components/ui";
import { useT } from "../i18n";
import { useApp } from "../store/app";
import { ChartTab } from "../study/ChartTab";
import { ControlTab, FeedbackTab, useDiagnosticsSpec } from "../study/ControlTab";
import { ScanTab } from "../study/ScanTab";
import { NoProject, PageHeader } from "./common";

type Tab = "scan" | "chart" | "control" | "feedback";

function readTab(): Tab {
  try {
    const v = localStorage.getItem("avas.study.tab");
    return v === "chart" || v === "control" || v === "feedback" ? v : "scan";
  } catch {
    return "scan";
  }
}

export default function StudyPage() {
  const t = useT();
  const project = useApp((s) => s.project);
  const [tab, setTab] = useState<Tab>(readTab);
  const { spec, setSpec, load } = useDiagnosticsSpec();
  if (!project.open) return <NoProject />;
  const choose = (v: Tab) => {
    setTab(v);
    try {
      localStorage.setItem("avas.study.tab", v);
    } catch {
      /* per-viewer convenience only */
    }
  };
  return (
    <div className="page">
      <div className="page-inner">
        <PageHeader
          title={t("Study")}
          hint={t("Scans, stability charts, response matrices and feedback on copies of the inputs: the project's InputFile and OutputFile are not changed. Engine runs are kept in Research/gui/, where scripts using avas.research find them too.")}
        />
        <Tabs
          value={tab}
          onChange={choose}
          className="study-tabs"
          tabs={[
            { value: "scan", label: t("Parameter scan"), icon: "graph-scatter" },
            { value: "chart", label: t("Stability chart"), icon: "symbol-color" },
            { value: "control", label: t("Response & controllability"), icon: "symbol-ruler" },
            { value: "feedback", label: t("Feedback"), icon: "sync" },
          ]}
        />
        <div style={{ display: tab === "scan" ? undefined : "none" }}>
          <ScanTab />
        </div>
        {tab === "chart" && <ChartTab />}
        {tab === "control" && <ControlTab spec={spec} setSpec={setSpec} reload={load} />}
        {tab === "feedback" && <FeedbackTab spec={spec} />}
      </div>
    </div>
  );
}
