import { cn } from "@/lib/utils";

export function TransitionBox({
  activationIndex,
  currentIndex,
  children,
  className,
}: {
  activationIndex: number;
  currentIndex: number;
  children: React.ReactNode;
  className?: string;
}) {
  const active = currentIndex === activationIndex;
  return (
    <div
      className={cn(
        "absolute inset-0 transition-all will-change-transform ease-out",
        active
          ? "translate-x-0 opacity-100 z-10"
          : activationIndex > currentIndex
            ? "translate-x-[30%] opacity-0 z-0 pointer-events-none"
            : "-translate-x-[30%] opacity-0 z-0 pointer-events-none",
        className
      )}
    >
      {children}
    </div>
  );
}
