export type ManualCategory = "guide" | "cases" | "reference";
export type ManualSection = { id: string; title: string; text: string; category: ManualCategory };

/** Only top-level chapter headings outside fences split documents. Explicit IDs survive translation. */
export function manualSections(source: string, category: ManualCategory): ManualSection[] {
  const result: ManualSection[] = [];
  let fence = "";
  let current: ManualSection | undefined;
  for (const line of source.replace(/^\uFEFF/, "").split(/\r?\n/)) {
    const marker = line.match(/^\s*(`{3,}|~{3,})/);
    if (marker) {
      if (!fence) fence = marker[1];
      else if (marker[1][0] === fence[0] && marker[1].length >= fence.length) fence = "";
    }
    const heading = !fence && line.match(/^#{1,2} (.+?)(?: \{#([\w-]+)\})?\s*$/);
    if (heading) {
      current = { id: heading[2] || `${category}-${result.length}`, title: heading[1], text: "", category };
      result.push(current);
    } else if (current) current.text += line + "\n";
  }
  return result;
}

/** A nonempty search spans every category, while browsing stays within the selected category. */
export function searchManual(sections: ManualSection[], category: ManualCategory, query: string) {
  const words = query.trim().toLocaleLowerCase().split(/\s+/).filter(Boolean);
  return sections.filter(s => words.length
    ? words.every(word => `${s.title}\n${s.text}`.toLocaleLowerCase().includes(word))
    : s.category === category);
}

/** Query terms for ranking: latin words and numbers, and Chinese runs plus their character pairs (the same rule as
 *  the Word-manual search in avas/ai/manual.py, so "GPU 计算" and "显卡驱动" both find sections without a tokenizer). */
export function manualTerms(query: string): string[] {
  const terms: string[] = [];
  for (const word of query.toLocaleLowerCase().match(/[a-z_][a-z_0-9']*|\d+(?:\.\d+)?|[一-鿿]+/g) ?? []) {
    if (/[一-鿿]/.test(word)) {
      terms.push(word);
      for (let i = 0; i + 1 < word.length; i++) terms.push(word.slice(i, i + 2));
    } else if (word.length > 1) terms.push(word);
  }
  return [...new Set(terms)];
}

/** Sections ranked for the assistant (title matches count three times, longer terms more), best first. */
export function rankManual(sections: ManualSection[], query: string, limit = 5): { section: ManualSection; score: number }[] {
  const terms = manualTerms(query);
  if (!terms.length) return [];
  const count = (text: string, term: string) => text.split(term).length - 1;
  return sections
    .map((section) => {
      const title = section.title.toLocaleLowerCase();
      const body = section.text.toLocaleLowerCase();
      const score = terms.reduce((sum, term) => sum + (count(body, term) + 3 * count(title, term)) * (term.length > 2 ? 3 : 1), 0);
      return { section, score };
    })
    .filter((h) => h.score > 0)
    .sort((a, b) => b.score - a.score)
    .slice(0, limit);
}
