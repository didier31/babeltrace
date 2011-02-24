/*
 * ctf-visitor-parent-links.c
 *
 * Common Trace Format Metadata Parent Link Creator.
 *
 * Copyright 2010 - Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <glib.h>
#include <inttypes.h>
#include <errno.h>
#include <babeltrace/list.h>
#include "ctf-scanner.h"
#include "ctf-parser.h"
#include "ctf-ast.h"

#define printf_dbg(fmt, args...)	fprintf(fd, "%s: " fmt, __func__, ## args)

static
int ctf_visitor_unary_expression(FILE *fd, int depth, struct ctf_node *node)
{
	int ret = 0;

	switch (node->u.unary_expression.link) {
	case UNARY_LINK_UNKNOWN:
	case UNARY_DOTLINK:
	case UNARY_ARROWLINK:
	case UNARY_DOTDOTDOT:
		break;
	default:
		fprintf(fd, "[error] %s: unknown expression link type %d\n", __func__,
			(int) node->u.unary_expression.link);
		return -EINVAL;
	}

	switch (node->u.unary_expression.type) {
	case UNARY_STRING:
	case UNARY_SIGNED_CONSTANT:
	case UNARY_UNSIGNED_CONSTANT:
		break;
	case UNARY_SBRAC:
		node->u.unary_expression.u.sbrac_exp->parent = node;
		ret = ctf_visitor_unary_expression(fd, depth + 1,
			node->u.unary_expression.u.sbrac_exp);
		if (ret)
			return ret;
		break;
	case UNARY_NESTED:
		node->u.unary_expression.u.nested_exp->parent = node;
		ret = ctf_visitor_unary_expression(fd, depth + 1,
			node->u.unary_expression.u.nested_exp);
		if (ret)
			return ret;
		break;

	case UNARY_UNKNOWN:
	default:
		fprintf(fd, "[error] %s: unknown expression type %d\n", __func__,
			(int) node->u.unary_expression.type);
		return -EINVAL;
	}
	return 0;
}

static
int ctf_visitor_type_specifier(FILE *fd, int depth, struct ctf_node *node)
{
	switch (node->u.type_specifier.type) {
	case TYPESPEC_VOID:
	case TYPESPEC_CHAR:
	case TYPESPEC_SHORT:
	case TYPESPEC_INT:
	case TYPESPEC_LONG:
	case TYPESPEC_FLOAT:
	case TYPESPEC_DOUBLE:
	case TYPESPEC_SIGNED:
	case TYPESPEC_UNSIGNED:
	case TYPESPEC_BOOL:
	case TYPESPEC_COMPLEX:
	case TYPESPEC_CONST:
	case TYPESPEC_ID_TYPE:
		break;

	case TYPESPEC_UNKNOWN:
	default:
		fprintf(fd, "[error] %s: unknown type specifier %d\n", __func__,
			(int) node->u.type_specifier.type);
		return -EINVAL;
	}
	return 0;
}

static
int ctf_visitor_type_declarator(FILE *fd, int depth, struct ctf_node *node)
{
	int ret = 0;
	struct ctf_node *iter;

	depth++;

	if (!cds_list_empty(&node->u.type_declarator.pointers)) {
		cds_list_for_each_entry(iter, &node->u.type_declarator.pointers,
					siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
	}

	switch (node->u.type_declarator.type) {
	case TYPEDEC_ID:
		break;
	case TYPEDEC_NESTED:
		if (node->u.type_declarator.u.nested.type_declarator) {
			node->u.type_declarator.u.nested.type_declarator->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1,
				node->u.type_declarator.u.nested.type_declarator);
			if (ret)
				return ret;
		}
		if (node->u.type_declarator.u.nested.length) {
			node->u.type_declarator.u.nested.length->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1,
				node->u.type_declarator.u.nested.length);
			if (ret)
				return ret;
		}
		if (node->u.type_declarator.bitfield_len) {
			node->u.type_declarator.bitfield_len = node;
			ret = ctf_visitor_parent_links(fd, depth + 1,
				node->u.type_declarator.bitfield_len);
			if (ret)
				return ret;
		}
		break;
	case TYPEDEC_UNKNOWN:
	default:
		fprintf(fd, "[error] %s: unknown type declarator %d\n", __func__,
			(int) node->u.type_declarator.type);
		return -EINVAL;
	}
	depth--;
	return 0;
}

int ctf_visitor_parent_links(FILE *fd, int depth, struct ctf_node *node)
{
	int ret = 0;
	struct ctf_node *iter;

	switch (node->type) {
	case NODE_ROOT:
		cds_list_for_each_entry(iter, &node->u.root._typedef,
					siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.root.typealias,
					siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.root.declaration_specifier, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.root.trace, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.root.stream, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.root.event, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;

	case NODE_EVENT:
		cds_list_for_each_entry(iter, &node->u.event.declaration_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_STREAM:
		cds_list_for_each_entry(iter, &node->u.stream.declaration_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_TRACE:
		cds_list_for_each_entry(iter, &node->u.trace.declaration_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;

	case NODE_CTF_EXPRESSION:
		depth++;
		cds_list_for_each_entry(iter, &node->u.ctf_expression.left, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.ctf_expression.right, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		depth--;
		break;
	case NODE_UNARY_EXPRESSION:
		return ctf_visitor_unary_expression(fd, depth, node);

	case NODE_TYPEDEF:
		depth++;
		cds_list_for_each_entry(iter, &node->u._typedef.declaration_specifier, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u._typedef.type_declarators, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		depth--;
		break;
	case NODE_TYPEALIAS_TARGET:
		depth++;
		cds_list_for_each_entry(iter, &node->u.typealias_target.declaration_specifier, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.typealias_target.type_declarators, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		depth--;
		break;
	case NODE_TYPEALIAS_ALIAS:
		depth++;
		cds_list_for_each_entry(iter, &node->u.typealias_alias.declaration_specifier, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.typealias_alias.type_declarators, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		depth--;
		break;
	case NODE_TYPEALIAS:
		node->u.typealias.target->parent = node;
		ret = ctf_visitor_parent_links(fd, depth + 1, node->u.typealias.target);
		if (ret)
			return ret;
		node->u.typealias.alias->parent = node;
		ret = ctf_visitor_parent_links(fd, depth + 1, node->u.typealias.alias);
		if (ret)
			return ret;
		break;

	case NODE_TYPE_SPECIFIER:
		ret = ctf_visitor_type_specifier(fd, depth, node);
		if (ret)
			return ret;
		break;
	case NODE_POINTER:
		break;
	case NODE_TYPE_DECLARATOR:
		ret = ctf_visitor_type_declarator(fd, depth, node);
		if (ret)
			return ret;
		break;

	case NODE_FLOATING_POINT:
		cds_list_for_each_entry(iter, &node->u.floating_point.expressions, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_INTEGER:
		cds_list_for_each_entry(iter, &node->u.integer.expressions, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_STRING:
		cds_list_for_each_entry(iter, &node->u.string.expressions, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_ENUMERATOR:
		if (node->u.enumerator.id)
			fprintf(fd, " id=\"%s\"", node->u.enumerator.id);
		fprintf(fd, ">\n");
		cds_list_for_each_entry(iter, &node->u.enumerator.values, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_ENUM:
		if (node->u._struct.name)
			fprintf(fd, "<enum name=\"%s\">\n",
				node->u._enum.enum_id);
		else
			fprintf(fd, "<enum >\n");
		depth++;

		if (node->u._enum.container_type) {
			node->u._enum.container_type->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, node->u._enum.container_type);
			if (ret)
				return ret;
		}

		cds_list_for_each_entry(iter, &node->u._enum.enumerator_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		depth--;
		break;
	case NODE_STRUCT_OR_VARIANT_DECLARATION:
		cds_list_for_each_entry(iter, &node->u.struct_or_variant_declaration.declaration_specifier, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		cds_list_for_each_entry(iter, &node->u.struct_or_variant_declaration.type_declarators, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_VARIANT:
		if (node->u.variant.choice)
			fprintf(fd, " choice=\"%s\"", node->u.variant.choice);
		cds_list_for_each_entry(iter, &node->u.variant.declaration_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;
	case NODE_STRUCT:
		cds_list_for_each_entry(iter, &node->u._struct.declaration_list, siblings) {
			iter->parent = node;
			ret = ctf_visitor_parent_links(fd, depth + 1, iter);
			if (ret)
				return ret;
		}
		break;

	case NODE_UNKNOWN:
	default:
		fprintf(fd, "[error] %s: unknown node type %d\n", __func__,
			(int) node->type);
		return -EINVAL;
	}
	return ret;
}