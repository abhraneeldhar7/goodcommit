import { useEffect, useState } from "react";

interface Scenario {
  cmd: string;
  options: string[];
  pick: number;
  hash: string;
  stats: string;
}

const SCENARIOS: Scenario[] = [
  {
    cmd: "fixed the login bug",
    options: [
      "fix(login): resolve login failure on invalid password",
      "fix(auth): improve failed-login error handling",
      "fix(login): redirect user after successful sign-in",
    ],
    pick: 1,
    hash: "8f3a2c1",
    stats: " 3 files changed, 42 insertions(+), 6 deletions(-)",
  },
  {
    cmd: "added pagination to the user list",
    options: [
      "feat(users): add pagination to user list",
      "feat(users): implement pagination for user list",
      "feat(ui): add pagination controls to user table",
    ],
    pick: 2,
    hash: "b74d9e0",
    stats: " 5 files changed, 148 insertions(+), 12 deletions(-)",
  },
  {
    cmd: "improved api error handling",
    options: [
      "refactor(api): centralize error handling",
      "fix(api): return consistent error responses",
      "refactor(api): improve error handling on requests",
    ],
    pick: 0,
    hash: "2ac55f3",
    stats: " 6 files changed, 90 insertions(+), 31 deletions(-)",
  },
  {
    cmd: "removed old analytics script",
    options: [
      "chore(analytics): remove legacy tracking script",
      "refactor(analytics): drop unused analytics code",
      "chore: remove deprecated analytics integration",
    ],
    pick: 2,
    hash: "e1c8d42",
    stats: " 2 files changed, 74 deletions(-)",
  },
  {
    cmd: "upgraded dependencies and fixed build",
    options: [
      "build(deps): upgrade dependencies and fix build",
      "chore(deps): bump dependency versions",
      "fix(build): resolve failures after dependency upgrade",
    ],
    pick: 1,
    hash: "09fbe7a",
    stats: " 9 files changed, 512 insertions(+), 118 deletions(-)",
  },
];

type Phase = "type" | "spin" | "select" | "done";

interface DemoState {
  scenario: number;
  phase: Phase;
  typed: string;
  spinFrame: number;
  selected: number;
  confirmed: boolean;
}

const INITIAL: DemoState = {
  scenario: 0,
  phase: "type",
  typed: "",
  spinFrame: 0,
  selected: 0,
  confirmed: false,
};

const GREEN = "#16bd0c";
const MUTED = "#555";
const FAINT = "#888";

const wait = (ms: number) => new Promise((r) => setTimeout(r, ms));

export default function Demo() {
  const [state, setState] = useState<DemoState>(INITIAL);

  useEffect(() => {
    let cancelled = false;

    async function run() {
      const s = SCENARIOS[state.scenario];
      if (!s) {
        await wait(600);
        if (!cancelled) setState(INITIAL);
        return;
      }

      if (state.phase === "type") {
        const cmd = 'xommit ' + s.cmd;
        if (state.typed.length < cmd.length) {
          await wait(45);
          if (!cancelled) setState((p) => ({ ...p, typed: cmd.slice(0, p.typed.length + 1) }));
        } else {
          await wait(400);
          if (!cancelled) setState((p) => ({ ...p, phase: "spin", spinFrame: 0 }));
        }
      } else if (state.phase === "spin") {
        await wait(80);
        if (!cancelled) setState((p) => ({ ...p, spinFrame: p.spinFrame + 1 }));
        if (state.spinFrame * 80 >= 1500) {
          if (!cancelled) setState((p) => ({ ...p, phase: "select", selected: 0 }));
        }
      } else if (state.phase === "select") {
        if (!state.confirmed) {
          if (state.selected < s.pick) {
            await wait(750);
            if (!cancelled) setState((p) => ({ ...p, selected: p.selected + 1 }));
          } else {
            await wait(800);
            if (!cancelled) setState((p) => ({ ...p, confirmed: true }));
          }
        } else {
          await wait(650);
          if (!cancelled) setState((p) => ({ ...p, phase: "done" }));
        }
      } else if (state.phase === "done") {
        await wait(2200);
        if (!cancelled) {
          const next = state.scenario + 1;
          if (next < SCENARIOS.length) {
            setState({ scenario: next, phase: "type", typed: "", spinFrame: 0, selected: 0, confirmed: false });
          } else {
            setState(INITIAL);
          }
        }
      }
    }

    run();
    return () => { cancelled = true; };
  }, [state]);

  const s = SCENARIOS[state.scenario];
  const cmd = s ? 'xommit ' + s.cmd : "";
  const spins = ["|", "/", "-", "\\"];

  return (
    <>
      <style>{`
        @keyframes gc-blink {
          0%, 49% { opacity: 1; }
          50%, 100% { opacity: 0; }
        }
      `}</style>
      <div aria-live="off">
        <div className="font-mono text-md leading-[1.75] whitespace-pre min-h-[16.5em]">
          {s && state.phase === "type" && (
            <div>
              <span style={{ color: GREEN }}>$ </span>
              {state.typed}
              <span className="inline-block w-[0.55em] h-[1.1em] ml-[2px] align-text-bottom bg-[#111] animate-[gc-blink_1.06s_steps(1)_infinite]" />
            </div>
          )}

          {s && state.phase === "spin" && (
            <>
              <div>
                <span style={{ color: GREEN }}>$ </span>
                {cmd}
              </div>
              <div style={{ color: MUTED }}>  {spins[state.spinFrame % 4]} Generating...</div>
            </>
          )}

          {s && (state.phase === "select" || state.phase === "done") && (
            <>
              <div>
                <span style={{ color: GREEN }}>$ </span>
                {cmd}
              </div>
              {s.options.map((opt, i) => {
                const isSelected = i === state.selected && !state.confirmed;
                const isChosen = i === s.pick && state.confirmed;
                if (isSelected) {
                  return (
                    <div key={i}>
                      <span style={{ color: GREEN }} className="font-semibold">{"  \u276F " + opt}</span>
                      <span className="inline-block w-[0.55em] h-[1.1em] ml-[2px] align-text-bottom bg-[#111] animate-[gc-blink_1.06s_steps(1)_infinite]" />
                    </div>
                  );
                }
                if (isChosen) {
                  return (
                    <div key={i} style={{ color: GREEN }} className="font-semibold">
                      {"  \u2714 " + opt}
                    </div>
                  );
                }
                if (state.confirmed && i === s.pick) return null;
                return (
                  <div key={i}>
                    {"    " + opt}
                  </div>
                );
              })}
              {!state.confirmed && (
                <div style={{ color: FAINT }}>{"  (\u2191\u2193 to navigate, Enter to confirm)"}</div>
              )}
              {state.confirmed && (
                <>
                  <div>
                    <span style={{ color: MUTED }}>[main {s.hash}]</span>
                    {" " + s.options[s.pick]}
                  </div>
                  <div>{s.stats}</div>
                </>
              )}
            </>
          )}
        </div>
      </div>
    </>
  );
}
