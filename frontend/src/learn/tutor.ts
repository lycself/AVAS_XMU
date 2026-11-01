// "Ask the tutor": hand a question about the primer to the AI assistant.  The
// assistant knows the primer through its primer_* tools (avas/ai/avas_tools.py);
// the page only opens the panel and sends the question.
import { reportError } from "../components/overlays";
import { sendMessage, setAssistantOpen, useAssistant } from "../assistant/store";
import { t } from "../i18n";

export function askTutor(question: string) {
  setAssistantOpen(true);
  if (useAssistant.getState().current?.busy) {
    reportError(new Error(t("The assistant is still answering; wait for it to finish.")));
    return;
  }
  void sendMessage(question);
}
