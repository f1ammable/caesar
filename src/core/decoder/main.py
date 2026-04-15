import re
import yaml
from jinja2 import Environment, FileSystemLoader
import asyncio
from playwright.async_api import async_playwright, Playwright
from collections import defaultdict

yaml_file = defaultdict(
     lambda: defaultdict(lambda: {"index_fields": {}, "entries": []})
 )

async def process_opcodes(op_row: list[list[str|int]]) -> list[dict[str, int | str]]:
    opcodes: list[dict[str, str | int]] = []
    pos: int = 31
    for op in op_row:
        if op[2]:
            lowBitPos: int = pos - (op[1]-1)
            opcodes.append({'name': op[2], 'high': pos, 'low': lowBitPos})
        pos -= op[1]
    return opcodes

async def make_yaml_table(instruction_list: list[list[str]], ops: list[dict[str, int | str]], first: bool) -> None:
    for insn in instruction_list:
        id: str = re.sub(r'[^0-9A-Za-z]+', '_', insn[-1]).strip('_').lower()
        bit_patterns = insn[:-1]
        if first:
            yaml_file["tables"]["primary"]["index_fields"] = ops
            yaml_file["tables"]["primary"]["entries"].append({ 'pattern': str(' '.join(bit_patterns)), 'handler': f"decode_{id}"})

    print(yaml.dump(dict(yaml_file)))

    

async def main():
    async with async_playwright() as p:
        browser = await p.chromium.launch(headless=True)
        page = await browser.new_page()
        await page.goto("https://developer.arm.com/documentation/ddi0602/2026-03/Index-by-Encoding?lang=en", wait_until='networkidle')
        opcodes: list[list[str|int]] = await page.evaluate(
"""
(() => {
    const opcodeList = [];
    for (const table of document.getElementsByTagName("table")) {
        const head = Array.from(table.getElementsByTagName('thead'));
        if (head.length == 0) { return; }

        const body = Array.from(table.getElementsByTagName('tbody'));
        const rows = Array.from(body[0].children);
        if (rows.length != 1) { return; }
        
        const bitPatterns = Array.from(rows[0].children);
        bitPatterns.forEach((e) => {
            opcodeList.push([e, e.colSpan ?? 0, e.innerText])
        });
        return opcodeList;
  }
})()
"""
        )

        instructions = await page.evaluate(
"""
(() => {
    const instructions = [];
    for (const table of document.getElementsByTagName("table")) {
        const head = Array.from(table.getElementsByTagName('thead'));
        if (head.length == 1) {continue}

        const body = Array.from(table.getElementsByTagName('tbody'));
        const rows = Array.from(body[0].children).filter((e, idx) => idx > 1);

        rows.forEach((e) => {
            const rowTable = Array.from(e.children);
            const rowData = []
            rowTable.forEach((x) => {
                rowData.push(x.innerText == '' ? 'x' : x.innerText)
            })
            instructions.push(rowData);
        })
    return instructions;
  }
})() 
"""
        )
        await make_yaml_table(instructions, await process_opcodes(opcodes), True)
        await browser.close()

asyncio.run(main())

# with open("armv8.yml", "r") as file:
#     data = yaml.safe_load(file)
#
# data = process_tables(data)
#
# env = Environment(loader=FileSystemLoader("."), trim_blocks=True, lstrip_blocks=True)
#
# template = env.get_template("decoder.j2")
