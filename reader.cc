#include <reader.h>
#include <alien.h>

#include <string>

namespace odc {

Reader::Reader(std::istream &rider): d_rider(rider), d_cancelled(false), d_readAlien(false), d_typeList(),
	d_state(new ReaderState()) {}

SHORTCHAR Reader::readSChar() {
	SHORTCHAR out;
	d_rider.read(&out, 1);
	return out;
}

BYTE Reader::readByte() {
	BYTE out;
	d_rider.read((char*)&out, 1);
	return out;
}

INTEGER Reader::readInt() {
	char *buf = new char[4];
	d_rider.read(buf, 4);
	if (isLittleEndian()) {
		return *(INTEGER *)buf;
	} else {
		char *out = new char[4];
		out[0] = buf[3]; out[1] = buf[2]; out[2] = buf[1]; out[3] = buf[0];
		return *(INTEGER *)out;
	}
}

void Reader::readSString(SHORTCHAR *out) {
	while (*out = readSChar()) {
		++out;
	}
}

INTEGER Reader::readVersion(INTEGER min, INTEGER max) {
	INTEGER version = readByte();
	//if (version < min || version > max) {
		// 			rd.TurnIntoAlien(alienVersion)
	//}
	return version;
}

Store* Reader::readStore() {
	SHORTCHAR kind = readSChar();
	if (kind == Store::NIL) {
		std::cout << "NIL STORE" << std::endl;
		return readNilStore();
	} else if (kind == Store::LINK) {
		std::cout << "LINK STORE" << std::endl;
		return readLinkStore();
	} else if (kind == Store::NEWLINK) {
		std::cout << "NEWLINK STORE" << std::endl;
		return readNewLinkStore();
	} else if (kind == Store::STORE) {
		std::cout << "STORE STORE" << std::endl;
		return readStoreOrElemStore(false);
	} else if (kind == Store::ELEM) {
		std::cout << "ELEM STORE" << std::endl;
		return readStoreOrElemStore(true);
	} else {
		throw 20;
	}
}
//	PROCEDURE (VAR rd: Reader) ReadStore* (OUT x: Store), NEW;
//		VAR a: Alien; t: Kernel.Type;
//			len, pos, pos1, id, comment, next, down, downPos, nextTypeId, nextElemId, nextStoreId: INTEGER;
//			kind: SHORTCHAR; path: TypePath; type: TypeName;
//			save: ReaderState;
Store *Reader::readNilStore() {
	return 0;
}
//		IF kind = nil THEN
//			rd.ReadInt(comment); rd.ReadInt(next);
//			rd.st.end := rd.Pos();
//			IF (next > 0) OR ((next = 0) & ODD(comment)) THEN rd.st.next := rd.st.end + next ELSE rd.st.next := 0 END;
//			x := NIL
Store *Reader::readLinkStore() {
	return 0;
}
//		ELSIF kind = link THEN
//			rd.ReadInt(id); rd.ReadInt(comment); rd.ReadInt(next);
//			rd.st.end := rd.Pos();
//			IF (next > 0) OR ((next = 0) & ODD(comment)) THEN rd.st.next := rd.st.end + next ELSE rd.st.next := 0 END;
//			x := ThisStore(rd.eDict, id)
Store *Reader::readNewLinkStore() {
	return 0;
}
//		ELSIF kind = newlink THEN
//			rd.ReadInt(id); rd.ReadInt(comment); rd.ReadInt(next);
//			rd.st.end := rd.Pos();
//			IF (next > 0) OR ((next = 0) & ODD(comment)) THEN rd.st.next := rd.st.end + next ELSE rd.st.next := 0 END;
//			x := ThisStore(rd.sDict, id)

Store *Reader::readStoreOrElemStore(bool isElem) {
	INTEGER id = isElem ? d_elemList.size() : d_storeList.size();
	TypePath path = readPath();
	std::cout << path.toString() << std::endl;
	const std::string &type = path[0];
	INTEGER comment = readInt();
	std::streampos pos1 = d_rider.tellg();
	std::streamoff next = readInt();
	std::streamoff down = readInt();
	std::streamoff len = readInt();
	std::streampos pos = d_rider.tellg();
	if (next > 0) {
		d_state->next = pos1 + next + (std::streamoff)4;
	} else {
		d_state->next = 0;
	}
	int downPos = 0;
	if (down > 0) {
		downPos = pos1 + down + (std::streamoff)8;
	}
	d_state->end = pos + len;
	d_cause = 0;
	// FIXME: insert whole bunch of checks here
//			ASSERT(len >= 0, 101);
//			IF next # 0 THEN
//				ASSERT(rd.st.next > pos1, 102);
//				IF down # 0 THEN
//					ASSERT(downPos < rd.st.next, 103)
//				END
//			END;
//			IF down # 0 THEN
//				ASSERT(downPos > pos1, 104);
//				ASSERT(downPos < rd.st.end, 105)
//			END;

	const TypeProxyBase *t = TypeRegister::getInstance().get(type); // FIXME type lookup here
	Store *x = 0;
	if (t != 0) {
		x = t->newInstance(id);
		x->internalize(*this);
//				x := NewStore(t); x.isElem := kind = elem
	} else {
//				rd.cause := thisTypeRes; AlienTypeReport(rd.cause, type);
//				x := NIL
	}

	if (x != 0) { // IF READING SUCCEEDS, INSERT MORE CHECKS HERE
	} else {
//				rd.SetPos(pos);
//				ASSERT(rd.cause # 0, 107);
		Alien *alien = new Alien(id, path); //, d_cause); //,file
		if (d_store == 0) {
			d_store = alien;
		} else {
			// join(d_store, alien)
			std::cout << "Man, should have written join(.,.)" << std::endl;
		}
		if (isElem) {
			d_elemList.push_back(alien);
		} else {
			d_storeList.push_back(alien);
		}
		ReaderState *save = d_state;
//				rd.nextTypeId := nextTypeId; rd.nextElemId := nextElemId; rd.nextStoreId := nextStoreId;
		internalizeAlien(alien, downPos, save->end);
		d_state = save;
//				ASSERT(rd.Pos() = rd.st.end, 108);
//				rd.cause := 0; rd.cancelled :=  FALSE; rd.readAlien := TRUE
		return alien;
	}

	return x;
}
//			t := ThisType(type);
//			IF t # NIL THEN
//				x := NewStore(t); x.isElem := kind = elem
//			ELSE
//				rd.cause := thisTypeRes; AlienTypeReport(rd.cause, type);
//				x := NIL
//			END;
//			IF x # NIL THEN
//				IF SamePath(t, path) THEN
//					IF kind = elem THEN
//						x.id := id; AddStore(rd.eDict, rd.eHead, x)
//					ELSE
//						x.id := id; AddStore(rd.sDict, rd.sHead, x)
//					END;
//					save := rd.st; rd.cause := 0; rd.cancelled :=  FALSE;
//					x.Internalize(rd);
//					rd.st := save;
//					IF rd.cause # 0 THEN x := NIL
//					ELSIF (rd.Pos() # rd.st.end) OR rd.rider.eof THEN
//						rd.cause := inconsistentVersion; AlienReport(rd.cause);
//						x := NIL
//					END
//				ELSE
//					rd.cause := inconsistentType; AlienTypeReport(rd.cause, type);
//					x := NIL
//				END
//			END;
//			
//			IF x # NIL THEN
//				IF rd.noDomain THEN
//					rd.store := x;
//					rd.noDomain := FALSE
//				ELSE
//					Join(rd.store, x)
//				END
//			ELSE	(* x is an alien *)
//				rd.SetPos(pos);
//				ASSERT(rd.cause # 0, 107);
//				NEW(a); a.path := path; a.cause := rd.cause; a.file := rd.rider.Base();
//				IF rd.noDomain THEN
//					rd.store := a;
//					rd.noDomain := FALSE
//				ELSE
//					Join(rd.store, a)
//				END;
//				IF kind = elem THEN
//					a.id := id; AddStore(rd.eDict, rd.eHead, a)
//				ELSE
//					a.id := id; AddStore(rd.sDict, rd.sHead, a)
//				END;
//				save := rd.st;
//				rd.nextTypeId := nextTypeId; rd.nextElemId := nextElemId; rd.nextStoreId := nextStoreId;
//				InternalizeAlien(rd, a.comps, downPos, pos, len);
//				rd.st := save;
//				x := a;
//				ASSERT(rd.Pos() = rd.st.end, 108);
//				rd.cause := 0; rd.cancelled :=  FALSE; rd.readAlien := TRUE
//			END


void Reader::internalizeAlien(Alien *alien, std::streampos down, std::streampos end) {
	std::streampos next = down != 0 ? down : end;
	while (d_rider.tellg() < end) {
		if (d_rider.tellg() < next) { // for some reason, this means its a piece (unstructured)
			std::cout << "Alien Piece" << std::endl;
			size_t len = next - d_rider.tellg();
			char *buf = new char[len];
			d_rider.read(buf, len);
			AlienComponent *comp = new AlienPiece(buf, len);
			alien->getComponents().push_back(comp);
		} else { // that means we've got a store
			std::cout << "Alien Store" << std::endl;
			d_rider.seekg(next);
			AlienComponent *comp = new AlienPart(readStore());
			alien->getComponents().push_back(comp);
			next = d_state->next > 0 ? d_state->next : end;
		}
	}
}

std::string &Reader::fixTypeName(std::string &name) {
	size_t pos = name.size() - 4;
	if (pos > 0 && name.substr(pos, 4).compare("Desc") == 0) {
		return name.replace(pos, 4, "^");
	}
	return name;
}

TypePath Reader::readPath() {
	TypePath path;
	SHORTCHAR kind = readSChar();
	SHORTCHAR *buf = new SHORTCHAR[64]; // TypeName has a maximum of length 64 (including terminator).
	int i;
	for (i = 0; kind == Store::NEWEXT; ++i) {
		readSString(buf);
		std::string name(buf);
		path.push_back(fixTypeName(name));
		addPathComponent(i == 0, path[i]);
//			IF path[i] # elemTName THEN INC(i) END;
		kind = readSChar();
	}

	if (kind == Store::NEWBASE) {
		readSString(buf);
		std::string name(buf);
		path.push_back(fixTypeName(name));
		addPathComponent(i == 0, path[i]);
		++i;
	} else if (kind == Store::OLDTYPE) {
		int id = readInt();
		if (i > 0) {
			d_typeList[d_typeList.size() - 1]->baseId = id;
		}
		while (id != -1) {
			path.push_back(d_typeList[id]->name);
			id = d_typeList[id]->baseId;
//				IF path[i] # elemTName THEN INC(i) END
			++i;
		}
	} else {
		throw 100;
	}
	return path;
}

void Reader::addPathComponent(bool first, const std::string &typeName) {
	int next = d_typeList.size();
	int curr = next - 1;
	if (!first) {
		d_typeList[curr]->baseId = next;
	}
	d_typeList.push_back(new TypeEntry(typeName));
}

} // namespace odc
