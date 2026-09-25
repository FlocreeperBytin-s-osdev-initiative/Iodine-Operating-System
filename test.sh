#!/usr/bin/env bash
set -e

echo "=== Running Iodine OS Automated Verification Test Suite ==="

node -e '
import path from "node:path";
import { V86 } from "./web/node_modules/v86-system/assets/libv86.mjs";

const config = {
    wasm_path: "./web/node_modules/v86-system/assets/v86.wasm",
    bios: { url: "./web/node_modules/v86-system/assets/seabios.bin" },
    fda: { url: path.resolve("bin/iodine.img") },
    boot_order: 0x01,
    autostart: true,
};

let output = "";
const emulator = new V86(config);

emulator.add_listener("serial0-output-byte", (byte) => {
    const ch = String.fromCharCode(byte);
    process.stdout.write(ch);
    output += ch;
});

const commands = [
    "echo [TEST] Iodine OS booted successfully!\n",
    "calc (100 - 25) * 4\n",
    "uname -a\n",
    "date\n",
    "uptime\n",
    "mem\n",
    "ps\n",
    "touch testfile.txt\n",
    "write testfile.txt Hello from Iodine Test Runner\n",
    "cat testfile.txt\n",
    "cat /etc/version\n",
    "fetch\n"
];

let idx = 0;
setTimeout(() => {
    const iv = setInterval(() => {
        if (idx < commands.length) {
            emulator.serial0_send(commands[idx++]);
        } else {
            clearInterval(iv);
            setTimeout(() => {
                emulator.destroy();

                console.log("\n--- Automated Verification Checks ---");
                const checks = [
                    { name: "Kernel Boot Splash", pattern: "IODINE OPERATING SYSTEM" },
                    { name: "PMM & VMM Init", pattern: "Virtual Memory Manager (Paging)" },
                    { name: "VFS Mounts", pattern: "Mounting ProcFS" },
                    { name: "Shell Prompt", pattern: "iodine:/home/user#" },
                    { name: "Calculator Test", pattern: "(100 - 25) * 4 = 300" },
                    { name: "Uname Output", pattern: "IodineOS iodine-pc 1.0.0" },
                    { name: "File Write/Cat", pattern: "Hello from Iodine Test Runner" },
                    { name: "Version File", pattern: "Iodine Operating System 1.0.0" },
                    { name: "ASCII Art Fetch", pattern: "Pure Custom Bare Metal" }
                ];

                let allPassed = true;
                for (const chk of checks) {
                    if (output.includes(chk.pattern)) {
                        console.log("  [PASS] " + chk.name);
                    } else {
                        console.error("  [FAIL] " + chk.name);
                        allPassed = false;
                    }
                }

                if (allPassed) {
                    console.log("\nALL VERIFICATION TESTS PASSED SUCCESSFULLY! ZERO ERRORS.\n");
                    process.exit(0);
                } else {
                    console.error("\nTEST FAILURES DETECTED.\n");
                    process.exit(1);
                }
            }, 2500);
        }
    }, 500);
}, 2500);
'
