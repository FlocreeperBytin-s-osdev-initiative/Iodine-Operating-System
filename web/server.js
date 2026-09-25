import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const rootDir = path.resolve(__dirname, '..');
const publicDir = path.resolve(__dirname, 'public');

const PORT = 3000;

const mimeTypes = {
    '.html': 'text/html',
    '.css': 'text/css',
    '.js': 'application/javascript',
    '.mjs': 'application/javascript',
    '.wasm': 'application/wasm',
    '.bin': 'application/octet-stream',
    '.img': 'application/octet-stream',
    '.iso': 'application/octet-stream',
    '.elf': 'application/octet-stream',
    '.json': 'application/json',
    '.png': 'image/png',
    '.svg': 'image/svg+xml',
    '.txt': 'text/plain'
};

const server = http.createServer((req, res) => {
    // CORS headers for all origins
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', '*');
    // Enable Cross-Origin Isolation for SharedArrayBuffer if needed
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');

    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    let pathname = decodeURIComponent(url.pathname);

    // API status endpoint
    if (pathname === '/api/info') {
        const info = {
            name: "Iodine Operating System",
            version: "1.0.0",
            architecture: "i386 (x86_32 Protected Mode)",
            kernel_type: "Monolithic",
            built_with: "GCC & GNU Binutils",
            memory: "128 MB RAM, 16 MB Heap",
            status: "Running"
        };
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(info, null, 2));
        return;
    }

    // Disk images download routes
    if (pathname.startsWith('/disk/')) {
        const filename = pathname.replace('/disk/', '');
        const filePath = path.join(rootDir, 'bin', filename);
        if (fs.existsSync(filePath)) {
            const stat = fs.statSync(filePath);
            res.writeHead(200, {
                'Content-Type': 'application/octet-stream',
                'Content-Length': stat.size,
                'Content-Disposition': `attachment; filename="${filename}"`
            });
            fs.createReadStream(filePath).pipe(res);
            return;
        }
    }

    // Static files
    if (pathname === '/') pathname = '/index.html';

    let filePath = path.join(publicDir, pathname);
    if (!fs.existsSync(filePath)) {
        // Fallback to bin/ if asking for iodine.img
        if (pathname === '/iodine.img') {
            filePath = path.join(rootDir, 'bin', 'iodine.img');
        } else {
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('404 Not Found');
            return;
        }
    }

    const ext = path.extname(filePath).toLowerCase();
    const contentType = mimeTypes[ext] || 'application/octet-stream';

    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Internal Server Error');
            return;
        }
        res.writeHead(200, {
            'Content-Type': contentType,
            'Content-Length': data.length
        });
        res.end(data);
    });
});

server.listen(PORT, '0.0.0.0', () => {
    console.log(`[Iodine OS] Web Emulator running at http://0.0.0.0:${PORT}`);
});
