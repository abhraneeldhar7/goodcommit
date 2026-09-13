import { useState } from "react";
import MarkdownRender from "./markdown-render/markdown-render";

import npmMd from "../content/install/npm.md?raw";
import curlMd from "../content/install/curl.md?raw";
import powershellMd from "../content/install/powershell.md?raw";
import manualMd from "../content/install/manual.md?raw";
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

  return (
    <div className="space-y-6">
      <div className="flex gap-1.5 items-center">
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

      <div className="relative h-full">
        {TABS.map((tab, i) => (
          <TransitionBox
            key={tab.id}
            activationIndex={i}
            currentIndex={TABS.findIndex(t => t.id === active)}
          >
            <MarkdownRender content={CONTENT[tab.id]} />
          </TransitionBox>
        ))}
      </div>
    </div>
  );
}
