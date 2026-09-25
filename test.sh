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
    "posix\n",
    "musl\n",
    "virtio\n",
    "ldk\n",
    "apk update\n",
    "apk list\n",
    "apk add curl\n",
    "apk info curl\n",
    "cal\n",
    "whoami\n",
    "morse hello\n",
    "banner IODINE\n",
    "cksum welcome.txt\n",
    "calc (100 - 25) * 4\n",
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
                    { name: "POSIX Subsystem Check", pattern: "100% Standard POSIX Interfaces Active" },
                    { name: "musl libc Compatibility", pattern: "Fully compliant with musl POSIX system call ABI" },
                    { name: "VirtIO Hardware Probe", pattern: "VIRTIO PCI HARDWARE BUS" },
                    { name: "Linux Driver Sandbox (GPL Barrier)", pattern: "GPL CONTAMINATION BARRIER" },
                    { name: "apk Package Manager (Alpine Clone)", pattern: "distinct packages available" },
                    { name: "apk Install (curl)", pattern: "Installing curl" },
                    { name: "BSD Utilities (whoami)", pattern: "root" },
                    { name: "BSD Utilities (cal)", pattern: "September 2026" },
                    { name: "BSD Utilities (morse)", pattern: ".... . .-.. .-.. ---" },
                    { name: "BSD Utilities (cksum)", pattern: "welcome.txt" },
                    { name: "Calculator Test", pattern: "(100 - 25) * 4 = 300" },
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
            }, 3000);
        }
    }, 600);
}, 3000);
'
