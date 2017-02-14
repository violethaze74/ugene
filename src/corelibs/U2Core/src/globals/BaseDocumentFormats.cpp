/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2017 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "BaseDocumentFormats.h"
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

const DocumentFormatId BaseDocumentFormats::ABIF("ABI");
const DocumentFormatId BaseDocumentFormats::ACE("ACE");
const DocumentFormatId BaseDocumentFormats::BAM("BAM");
const DocumentFormatId BaseDocumentFormats::BED("BED");
const DocumentFormatId BaseDocumentFormats::CLUSTAL_ALN("CLUSTAL");
const DocumentFormatId BaseDocumentFormats::DATABASE_CONNECTION("database_connection");
const DocumentFormatId BaseDocumentFormats::DIFF("Diff");
const DocumentFormatId BaseDocumentFormats::FASTA("FASTA");
const DocumentFormatId BaseDocumentFormats::FASTQ("FASTQ");
const DocumentFormatId BaseDocumentFormats::FPKM_TRACKING_FORMAT("FPKM-Tracking");
const DocumentFormatId BaseDocumentFormats::GFF("GFF");
const DocumentFormatId BaseDocumentFormats::GTF("GTF");
const DocumentFormatId BaseDocumentFormats::INDEX("Index");
const DocumentFormatId BaseDocumentFormats::MEGA("MEGA");
const DocumentFormatId BaseDocumentFormats::MSF("MSF");
const DocumentFormatId BaseDocumentFormats::NEWICK("Newick");
const DocumentFormatId BaseDocumentFormats::NEXUS("Nexus");
const DocumentFormatId BaseDocumentFormats::PDW("PDW");
const DocumentFormatId BaseDocumentFormats::PHYLIP_INTERLEAVED("PHYLIP-Interleaved");
const DocumentFormatId BaseDocumentFormats::PHYLIP_SEQUENTIAL("PHYLIP-Sequential");
const DocumentFormatId BaseDocumentFormats::PLAIN_ASN("MMDB");
const DocumentFormatId BaseDocumentFormats::PLAIN_EMBL("EMBL");
const DocumentFormatId BaseDocumentFormats::PLAIN_GENBANK("Genbank");
const DocumentFormatId BaseDocumentFormats::PLAIN_PDB("PDB");
const DocumentFormatId BaseDocumentFormats::PLAIN_SWISS_PROT("Swiss-Prot");
const DocumentFormatId BaseDocumentFormats::PLAIN_TEXT("Text");
const DocumentFormatId BaseDocumentFormats::RAW_DNA_SEQUENCE("Raw");
const DocumentFormatId BaseDocumentFormats::SAM("SAM");
const DocumentFormatId BaseDocumentFormats::SCF("SCF");
const DocumentFormatId BaseDocumentFormats::SNP("SNP");
const DocumentFormatId BaseDocumentFormats::SRF("SRFASTA");
const DocumentFormatId BaseDocumentFormats::STOCKHOLM("Stockholm");
const DocumentFormatId BaseDocumentFormats::UGENEDB("Usqlite");
const DocumentFormatId BaseDocumentFormats::VCF4("VCF");
const DocumentFormatId BaseDocumentFormats::VECTOR_NTI_SEQUENCE("Vector NTI Sequence");

DocumentFormat* BaseDocumentFormats::get(const DocumentFormatId& formatId) {
    return AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
}


}//namespace
