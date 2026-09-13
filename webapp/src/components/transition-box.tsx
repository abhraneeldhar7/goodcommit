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
        "absolute inset-0 w-full h-full transition-all will-change-all ease-out",
        active
          ? "translate-x-0 opacity-100"
          : activationIndex > currentIndex
            ? "translate-x-[30%] opacity-0"
            : "-translate-x-[30%] opacity-0",
        className
      )}
    >
      {children}
    </div>
  );
}
