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
      "fix(auth): improve failed-login error handling",
      "fix(login): resolve login failure on invalid password",
      "fix(login): redirect user after successful sign-in",
    ],
    pick: 2,
    hash: "8f3a2c1",
    stats: " 3 files changed, 42 insertions(+), 6 deletions(-)",
  },
  {
    cmd: "added dark mode toggle",
    options: [
      "feat(ui): add dark mode toggle to settings",
      "feat(theme): implement dark mode support",
      "feat(ui): add theme switcher component",
    ],
    pick: 1,
    hash: "a1b2c3d",
    stats: " 4 files changed, 87 insertions(+), 12 deletions(-)",
  },
  {
    cmd: "fixed the api timeout",
    options: [
      "fix(api): add request timeout handling",
      "fix(server): resolve timeout on slow connections",
      "fix(api): set 30s timeout for external requests",
    ],
    pick: 0,
    hash: "d4e5f6a",
    stats: " 2 files changed, 31 insertions(+), 5 deletions(-)",
  },
  {
    cmd: "wrote tests for user service",
    options: [
      "test(user): add unit tests for user service",
      "test: add user service test coverage",
      "test(user): write tests for create and update",
    ],
    pick: 2,
    hash: "b7c8d9e",
    stats: " 1 file changed, 156 insertions(+)",
  },
  {
    cmd: "added pagination to the user list",
    options: [
      "feat(users): add pagination to user list",
      "feat(ui): add pagination controls to user table",
      "feat(users): implement pagination for user list",
    ],
    pick: 1,
    hash: "c1d2e3f",
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
    cmd: "added search to the dashboard",
    options: [
      "feat(search): add search bar to dashboard",
      "feat(dashboard): implement global search",
      "feat(ui): add search with autocomplete",
    ],
    pick: 1,
    hash: "f4a5b6c",
    stats: " 7 files changed, 203 insertions(+), 18 deletions(-)",
  },
  {
    cmd: "fixed memory leak in websocket",
    options: [
      "fix(ws): cleanup stale connections on disconnect",
      "fix(network): resolve memory leak in websocket handler",
      "fix(ws): close idle connections after timeout",
    ],
    pick: 0,
    hash: "7d8e9f0",
    stats: " 3 files changed, 29 insertions(+), 41 deletions(-)",
  },
  {
    cmd: "updated readme with install steps",
    options: [
      "docs: update readme with installation steps",
      "docs(readme): add setup instructions",
      "chore: update readme with quickstart guide",
    ],
    pick: 0,
    hash: "a0b1c2d",
    stats: " 1 file changed, 34 insertions(+), 11 deletions(-)",
  },
  {
    cmd: "made the navbar responsive",
    options: [
      "fix(ui): make navbar responsive on mobile",
      "feat(responsive): add mobile nav drawer",
      "style(nav): add responsive breakpoints to navbar",
    ],
    pick: 0,
    hash: "e3f4a5b",
    stats: " 3 files changed, 67 insertions(+), 22 deletions(-)",
  },
  {
    cmd: "added rate limiting to api",
    options: [
      "feat(api): add rate limiting middleware",
      "security(api): implement request throttling",
      "feat(security): add rate limit per user",
    ],
    pick: 1,
    hash: "6c7d8e9",
    stats: " 4 files changed, 112 insertions(+), 8 deletions(-)",
  },
];

type Phase = "type" | "spin" | "select" | "done";

interface DemoState {
  scenario: number;
  phase: Phase;
  typed: string;
  spinFrame: number;
  selected: number;
}

const INITIAL: DemoState = {
  scenario: 0,
  phase: "type",
  typed: "",
  spinFrame: 0,
  selected: 0,
};

const GREEN = "#16bd0c";
const MUTED = "#555";
const FAINT = "#888";

const wait = (ms: number) => new Promise((r) => setTimeout(r, ms));

export default function CLIDemoSection() {
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
        if (state.selected < s.pick) {
          await wait(750);
          if (!cancelled) setState((p) => ({ ...p, selected: p.selected + 1 }));
        } else {
          await wait(800);
          if (!cancelled) setState((p) => ({ ...p, phase: "done" }));
        }
      } else if (state.phase === "done") {
        await wait(2200);
        if (!cancelled) {
          const next = state.scenario + 1;
          if (next < SCENARIOS.length) {
            setState({ scenario: next, phase: "type", typed: "", spinFrame: 0, selected: 0 });
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
        <div className="font-mono text-base sm:text-md leading-[1.75] whitespace-pre h-[7rem] w-full">
          {s && state.phase === "type" && (
            <div className="truncate">
              <span style={{ color: GREEN }}>$ </span>
              {state.typed}
              <span className="inline-block w-[0.55em] h-[1.1em] ml-[2px] align-text-bottom bg-[#111] animate-[gc-blink_1.06s_steps(1)_infinite]" />
            </div>
          )}

          {s && state.phase === "spin" && (
            <>
              <div className="truncate">
                <span style={{ color: GREEN }}>$ </span>
                {cmd}
              </div>
              <div className="truncate" style={{ color: MUTED }}>  {spins[state.spinFrame % 4]} Generating...</div>
            </>
          )}

          {s && (state.phase === "select" || state.phase === "done") && (
            <>
              <div className="truncate">
                <span style={{ color: GREEN }}>$ </span>
                {cmd}
              </div>
              {state.phase === "select" && s.options.map((opt, i) => {
                if (i === state.selected) {
                  return (
                    <div key={i} className="truncate">
                      <span style={{ color: GREEN }} className="font-medium">{"  \u276F " + opt}</span>
                      <span className="inline-block w-[0.55em] h-[1.1em] ml-[2px] align-text-bottom bg-[#111] animate-[gc-blink_1.06s_steps(1)_infinite]" />
                    </div>
                  );
                }
                return (
                  <div key={i} className="truncate">
                    {"    " + opt}
                  </div>
                );
              })}
              {state.phase === "select" && (
                <div className="truncate" style={{ color: FAINT }}>{"  (\u2191\u2193 to navigate, Enter to confirm)"}</div>
              )}
              {state.phase === "done" && (
                <>
                  <div className="truncate">
                    <span style={{ color: MUTED }}>[main {s.hash}]</span>
                    {" " + s.options[s.pick]}
                  </div>
                  <div className="truncate">{s.stats}</div>
                </>
              )}
            </>
          )}
        </div>
      </div>
    </>
  );
}
