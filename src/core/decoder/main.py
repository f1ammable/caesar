import re
import yaml
from jinja2 import Environment, FileSystemLoader
from playwright.sync_api import Page, sync_playwright
from collections import defaultdict

yaml_file = defaultdict(
    lambda: defaultdict(lambda: {"index_fields": {}, "entries": []})
)

queued_links: dict[str, bool] = defaultdict()


def scrape_opcode_table(p: Page) -> list[dict[str, int | str]]:
    tables: list[list[list[str | int]]] = p.evaluate(
        """
(() => {
    const opcodes = [];
    for (const table of document.getElementsByTagName("table")) {
        const head = Array.from(table.getElementsByTagName('thead'));
        if (head.length == 0) { continue; }

        const body = Array.from(table.getElementsByTagName('tbody'));
        const rows = Array.from(body[0].children);
        if (rows.length != 1) { continue; }
        
        const tableOpcodes = []
        const bitPatterns = Array.from(rows[0].children);
        bitPatterns.forEach((e) => {
            tableOpcodes.push([e, e.colSpan ?? 0, e.innerText])
        });
        opcodes.push(tableOpcodes)
  }
    return opcodes;
})() 
        """
    )
    opcodes: list[dict[str, str | int]] = []
    pos: int = 31
    for table in tables:
        for bits in table:
            if bits[2]:
                lowBitPos: int = pos - (bits[1] - 1)
                opcodes.append({"name": bits[2], "high": pos, "low": lowBitPos})
            pos -= bits[1]
    return opcodes


def scrape_primary_instruction_table(p: Page) -> list[list[str]]:
    data: list[list[str]] = p.evaluate(
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
				const linkField = rowTable.at(-1).children[0]
				if (linkField == null) {rowData.push("")}
				else {rowData.push(linkField.href)}
                instructions.push(rowData);
            })
        return instructions;
      }
    })() 
        """
    )

    for x in data:
        queued_links[x[-1]] = False
        x.pop()

    return data

# TODO: Maybe (hopefully) we don't need a separate function for this
def scrape_secondary_instruction_table(p: Page) -> list[list[str]]:
    pass


def add_to_yaml(
    instruction_list: list[list[str]], ops: list[dict[str, int | str]], first: bool
) -> None:
    for insn in instruction_list:
        id: str = re.sub(r"[^0-9A-Za-z]+", "_", insn[-1]).strip("_").lower()
        bit_patterns = insn[:-1]
        if first:
            yaml_file["tables"]["primary"]["index_fields"] = ops
            yaml_file["tables"]["primary"]["entries"].append(
                {"pattern": str(" ".join(bit_patterns)), "handler": f"decode_{id}"}
            )
        else:
            yaml_file["tables"][id]["index_fields"] = ops
            yaml_file["tables"][id]["entries"].append(
                {"pattern": str(" ".join(bit_patterns)), "handler": f"decode_{id}"}
            )

    print(yaml.dump(dict(yaml_file)))


def main():
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page()
        page.goto(
            "https://developer.arm.com/documentation/ddi0602/2026-03/Index-by-Encoding?lang=en",
            wait_until="networkidle",
        )

        opcodes: list[dict[str, int | str]] = scrape_opcode_table(page)
        instructions: list[list[str]] = scrape_primary_instruction_table(page)
        add_to_yaml(instructions, opcodes, True)

        # for link in queued_links:
        #     page.goto(link)
        #     opcodes = scrape_opcode_table(page)
        #     instructions = scrape_secondary_instruction_table(page)
        #     add_to_yaml(instructions, opcodes, False)

        browser.close()


main()

# with open("armv8.yml", "r") as file:
#     data = yaml.safe_load(file)
#
# data = process_tables(data)
#
# env = Environment(loader=FileSystemLoader("."), trim_blocks=True, lstrip_blocks=True)
#
# template = env.get_template("decoder.j2")
