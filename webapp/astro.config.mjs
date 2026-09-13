// @ts-check
import { defineConfig } from 'astro/config';
import react from '@astrojs/react';
import tailwindcss from "@tailwindcss/vite";
import path from 'path';

// https://astro.build/config
export default defineConfig({
    integrations: [react()],
    server: { port: 3000, allowedHosts: ["yearlong-jon-patrilineal.ngrok-free.dev"] },
    vite: {
        plugins: [tailwindcss()],
        resolve: {
            alias: {
                '@': path.resolve('./src'),
            },
        },
    },
});
