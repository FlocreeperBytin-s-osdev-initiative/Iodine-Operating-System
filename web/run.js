#!/usr/bin/env node
import path from "node:path";
import { fileURLToPath } from "node:url";
import { V86 } from "./node_modules/v86-system/assets/libv86.mjs";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, "..");

const config = {
    wasm_path: path.join(__dirname, "public/v86.wasm"),
    bios: { url: path.join(__dirname, "public/seabios.bin") },
    fda: { url: path.join(rootDir, "bin/iodine.img") },
    boot_order: 0x01,
    autostart: true,
};

const emulator = new V86(config);

emulator.add_listener("serial0-output-byte", (b) => {
    process.stdout.write(String.fromCharCode(b));
});

function cleanup() {
    if (process.stdin.isTTY) {
        process.stdin.setRawMode(false);
    }
    emulator.destroy();
    process.exit(0);
}

process.on("SIGTERM", cleanup);
process.on("SIGINT", cleanup);

if (process.stdin.isTTY) {
    process.stdin.setRawMode(true);
}
process.stdin.resume();
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => {
    emulator.serial0_send(chunk);
});
