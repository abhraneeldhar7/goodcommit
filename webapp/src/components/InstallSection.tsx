import { useState } from "react";
import MarkdownRender from "./markdown-render/markdown-render";

import npmMd from "../content/npm.md?raw";
import curlMd from "../content/curl.md?raw";
import powershellMd from "../content/powershell.md?raw";
import manualMd from "../content/manual.md?raw";
import { TransitionBox } from "./transition-box";

const TABS = [
  { id: "npm", label: "npm" },
  { id: "curl", label: "curl" },
  { id: "powershell", label: "PowerShell" },
  { id: "manual", label: "Manual" },
] as const;

const CONTENT: Record<string, string> = {
  npm: npmMd,
  curl: curlMd,
  powershell: powershellMd,
  manual: manualMd,
};

export default function InstallSection() {
  const [active, setActive] = useState<"npm" | "curl" | "powershell" | "manual">("npm");
  const activeIndex = TABS.findIndex(t => t.id === active);

  return (
    <div className="space-y-4 mt-10">
      <div className="flex gap-1.5 items-center sticky top-[57px] sm:top-[50px] z-15 bg-background py-1">
      <span className="leading-[0.5em] text-[1.5rem] text-git mr-2">{">"}</span>
        {TABS.map((tab, index) => (
          <button
            key={index}
            type="button"
            onClick={() => setActive(tab.id)}
            className={`transition-all hover:bg-muted rounded-xs py-1 px-2.5 border-1 ${active===tab.id?"border-border bg-muted/50":"border-transparent"}`}
          >
            {tab.label}
          </button>
        ))}
      </div>

      <div className="relative overflow-hidden">
        <div className="invisible">
          <MarkdownRender content={CONTENT[active]} />
        </div>
        {TABS.map((tab, i) => (
          <TransitionBox
            key={tab.id}
            activationIndex={i}
            currentIndex={activeIndex}
          >
            <MarkdownRender content={CONTENT[tab.id]} />
          </TransitionBox>
        ))}
      </div>
    </div>
  );
}
