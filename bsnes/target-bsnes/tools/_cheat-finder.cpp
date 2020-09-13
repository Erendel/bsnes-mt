auto CheatFinder::create() -> void {
	setCollapsible();
	setVisible(false);

	searchList.setHeadered();

	searchList.onActivate([&](auto cell) {
		auto item = searchList.selected();

		if (!item) {
			return;
		}

		uint address  = toHex(item.cell(0).text().trimLeft("0x", 1L));
		string data   = item.cell(1).text().trimLeft("0x", 1L).split(" ", 1L).first();
		auto dataSize = data.size();

		string code;

		if (dataSize == 2) {
			code.append(hex(address + 0, 6L), "=", data.slice(0, 2), "\n");
		}
		else if (dataSize == 4) {
			code.append(hex(address + 0, 6L), "=", data.slice(2, 2), "\n");
			code.append(hex(address + 1, 6L), "=", data.slice(0, 2), "\n");
		}
		else if (dataSize == 6) {
			code.append(hex(address + 0, 6L), "=", data.slice(4, 2), "\n");
			code.append(hex(address + 1, 6L), "=", data.slice(2, 2), "\n");
			code.append(hex(address + 2, 6L), "=", data.slice(0, 2), "\n");
		}

		toolsWindow.show(1);
		cheatEditor.addButton.doActivate();
		cheatWindow.codeValue.setText(code).doChange();
	});

	searchValue.onActivate([&] {
		eventScan();
	});

	searchLabel.setText({bmt::get("Tools.CheatFinder.Value").data(), ':'});

	searchSize.append(ComboButtonItem().setText("Byte"));
	searchSize.append(ComboButtonItem().setText("Word"));
	searchSize.append(ComboButtonItem().setText("Long"));

	searchMode.append(ComboButtonItem().setText("="));
	searchMode.append(ComboButtonItem().setText("!="));
	searchMode.append(ComboButtonItem().setText(">="));
	searchMode.append(ComboButtonItem().setText("<="));
	searchMode.append(ComboButtonItem().setText(">"));
	searchMode.append(ComboButtonItem().setText("<"));

	searchSpan.append(ComboButtonItem().setText("WRAM"));
	searchSpan.append(ComboButtonItem().setText(bmt::get("Tools.CheatFinder.All").data()));

	searchScan.setText(bmt::get("Tools.CheatFinder.Scan").data()).onActivate([&] {
		eventScan();
	});

	searchClear.setText(bmt::get("Common.Clear").data()).onActivate([&] {
		eventClear();
	});

	refresh();
}

auto CheatFinder::restart() -> void {
	searchValue.setText("");
	searchSize.items().first().setSelected();
	searchMode.items().first().setSelected();
	searchSpan.items().first().setSelected();
	candidates.reset();
	refresh();
}

auto CheatFinder::refresh() -> void {
	searchList.reset();
	searchList.append(TableViewColumn().setText(bmt::get("Tools.CheatFinder.Address").data()));
	searchList.append(TableViewColumn().setText(bmt::get("Tools.CheatFinder.Value").data()));

	for (auto& candidate : candidates) {
		TableViewItem item{&searchList};
		item.append(TableViewCell().setText({"0x", hex(candidate.address, 6L)}));

		if (candidate.size == 0) {
			item.append(TableViewCell().setText({"0x", hex(candidate.data, 2L), " (", candidate.data, ")"}));
		}

		if (candidate.size == 1) {
			item.append(TableViewCell().setText({"0x", hex(candidate.data, 4L), " (", candidate.data, ")"}));
		}

		if (candidate.size == 2) {
			item.append(TableViewCell().setText({"0x", hex(candidate.data, 6L), " (", candidate.data, ")"}));
		}
	}

	for (uint n : range(2)) {
		Application::processEvents();
		searchList.resizeColumns();
	}
}

auto CheatFinder::eventScan() -> void {
	uint32_t size = searchSize.selected().offset();
	uint32_t mode = searchMode.selected().offset();
	uint32_t span = searchSpan.selected().offset();
	uint32_t data = searchValue.text().replace("$", "0x").replace("#", "").integer();

	if (size == 0) {
		data &= 0xff;
	}
	else if (size == 1) {
		data &= 0xffff;
	}
	else if (size == 2) {
		data &= 0xffffff;
	}

	if (candidates) {
		vector<CheatCandidate> result;

		for (auto& candidate : candidates) {
			uint address = candidate.address;

			if ((address & 0x40e000) == 0x000000) {  //00-3f,80-bf:0000-1fff (WRAM mirrors)
				continue;
			}

			if ((address & 0x40e000) == 0x002000) {  //00-3f,80-bf:2000-3fff (I/O)
				continue;
			}

			if ((address & 0x40e000) == 0x004000) {  //00-3f,80-bf:4000-5fff (I/O)
				continue;
			}

			if (span == 0 && (address < 0x7e0000 || address > 0x7fffff)) {
				continue;
			}

			auto value = read(size, address);

			if (compare(mode, value, data)) {
				result.append({address, value, size, mode, span});

				if (result.size() >= 4096) {
					break;
				}
			}
		}

		candidates = result;
	}
	else {
		for (uint address : range(1 << 24)) {
			if ((address & 0x40e000) == 0x000000) {  //00-3f,80-bf:0000-1fff (WRAM mirrors)
				continue;
			}

			if ((address & 0x40e000) == 0x002000) {  //00-3f,80-bf:2000-3fff (I/O)
				continue;
			}

			if ((address & 0x40e000) == 0x004000) {  //00-3f,80-bf:4000-5fff (I/O)
				continue;
			}

			if (span == 0 && (address < 0x7e0000 || address > 0x7fffff)) {
				continue;
			}

			auto value = read(size, address);

			if (compare(mode, value, data)) {
				candidates.append({address, value, size, mode, span});

				if (candidates.size() >= 4096) {
					break;
				}
			}
		}
	}

	refresh();
}

auto CheatFinder::eventClear() -> void {
	candidates.reset();
	refresh();
}

auto CheatFinder::read(uint32_t size, uint32_t address) -> uint32_t {
	uint32_t value = 0;

	if (size >= 0) {
		value |= emulator->read(address + 0) <<  0;
	}

	if (size >= 1) {
		value |= emulator->read(address + 1) <<  8;
	}

	if (size >= 2) {
		value |= emulator->read(address + 2) << 16;
	}

	return value;
}

auto CheatFinder::compare(uint32_t mode, uint32_t x, uint32_t y) -> bool {
	if (mode == 0) {
		return x == y;
	}

	if (mode == 1) {
		return x != y;
	}

	if (mode == 2) {
		return x >= y;
	}

	if (mode == 3) {
		return x <= y;
	}

	if (mode == 4) {
		return x >  y;
	}

	if (mode == 5) {
		return x <  y;
	}

	return false;
}